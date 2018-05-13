
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>

#define str(x) __str(x)
#define __str(x) #x

#include "version.h"
#include "document.h"
#include "html.h"
#include "latex.h"

//#include "common.h"
#include "utils.h"
#include <time.h>


/* FEATURES INFO / DEFAULTS */

enum renderer_type {
	RENDERER_HTML,
	RENDERER_LATEX,
	RENDERER_HTML_TOC
};

#define DEF_IUNIT 1024
#define DEF_OUNIT 64
#define DEF_MAX_NESTING 16

struct option_data {
    char *basename;
    int done;

    /* time reporting */
    int show_time;

    /* I/O */
    size_t iunit;
    size_t ounit;
    const char *filename;

    /* renderer */
    enum renderer_type renderer;
    int toc_level;
    scidown_render_flags render_flags;

    /* parsing */
    hoedown_extensions extensions;
    size_t max_nesting;
};

static const char *module = "md2html";

static const char *usr_input = 0;
static const char *OutputFile = 0;
static const char *headFile = 0;
static char *headContents = 0;
//////////////////////////////////////////////////////
/// utils
#ifdef _WIN32
#define M_IS_DIR _S_IFDIR
#else // !_WIN32
#define M_IS_DIR S_IFDIR
#endif

#define MDT_NONE 0
#define MDT_FILE 1
#define MDT_DIR  2

static struct stat buf;
static int is_file_or_directory(const char * path)
{
    if (!path)
        return MDT_NONE;
    if (stat(path, &buf) == 0)
    {
        if (buf.st_mode & M_IS_DIR)
            return MDT_DIR;
        else
            return MDT_FILE;
    }
    return MDT_NONE;
}

static size_t get_last_file_size() { return buf.st_size; }
//////////////////////////////////////////////////////
void print_version()
{
    printf("Built with Hoedown " HOEDOWN_VERSION ".\n");
}

void give_help(char *name)
{
    printf("%s: usage: [options] usr_input\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(0)\n");
    printf(" --out file    (-o) = Write output to this file.\n");
    printf(" --HEAD file   (-H) = Get 'doctype' and <head>...</head> from file.\n");
    printf(" --version     (-v) = Show library version, and exit(0)\n");
    printf("\n  ");
    print_version();
    printf("\n");
    printf("  User input file is assumed to be a 'markdown' coded file, and\n");
    printf("  the 'Hoedown' library will convert it to HTML, output to stdout\n");
    printf("  unless an output file name is given\n");
}

int parse_args(int argc, char **argv)
{
    int i, i2, c;
    char *arg, *sarg;
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        i2 = i + 1;
        if (*arg == '-') {
            sarg = &arg[1];
            while (*sarg == '-')
                sarg++;
            c = *sarg;
            switch (c) {
            case 'h':
            case '?':
                give_help(argv[0]);
                return 2;
                break;
            case 'v':
                print_version();
                return 2;
            case 'o':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    if (!OutputFile) {
                        OutputFile = strdup(sarg);
                    }
                    else {
                        fprintf(stderr, "Too many parameters. Already had out '%s',"
                            " found '%s' too. Use -? for help.\n", OutputFile, sarg);
                        return 1;
                    }
                }
                else {
                    fprintf(stderr, "Option '%s' must be followed by output file, use -? for help.\n", arg);
                }
                break;
            case 'H':   /* get users HTML HEADER file */
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    if (is_file_or_directory(sarg) == MDT_FILE) {
                        if (headFile) {
                            fprintf(stderr, "Too many parameters. Already had out '%s', found '%s %s' too. Use -? for help.\n", headFile, arg, sarg);
                        }
                        else {
                            size_t len = get_last_file_size();
                            headContents = new char[len + 1];    // allocate memory for a buffer of appropriate dimension
                            if (!headContents) {
                                fprintf(stderr, "Error: Unable to allocate %u bytes!\n", (unsigned int)(len + 1));
                                return 1;
                            }
                            memset(headContents, 0, len + 1);   /* is this really required? But have 'seen' some rubbish! */
                            std::ifstream t;
                            t.open(sarg);
                            if (t.bad()) {
                                fprintf(stderr, "Error: Unable to 'open' %s\n", sarg);
                                return 1;
                            }
                            t.read(headContents, len);    // read the whole file into the buffer
                            t.close();                    // close file handle
                            headContents[len] = 0;
                            headFile = sarg;
                        }
                    }
                    else {
                        fprintf(stderr, "Error: Unable to 'stat' file '%s'\n", sarg);
                    }
                }
                else {
                    fprintf(stderr, "Option '%s' must be followed by html header file, use -? for help.\n", arg);
                }
                break;
            default:
                fprintf(stderr, "%s: Unknown argument '%s'. Try -? for help...\n", module, arg);
                return 1;
            }
        }
        else {
            // bear argument
            if (usr_input) {
                fprintf(stderr, "%s: Already have input '%s'! What is this '%s'?\n", module, usr_input, arg);
                return 1;
            }
            usr_input = strdup(arg);
        }
    }
    if (!usr_input) {
        fprintf(stderr, "%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

int parse_options(int argc, char **argv, void *opaque)
{
    struct option_data *data = (struct option_data *)opaque;
    int res = parse_args(argc, argv);
    data->basename = argv[0];
    data->filename = usr_input;
    if (res) {
        if (res == 2)
            data->done = 1;
        return 0;
    }
    return 1;
}

/* Get local info */
localization get_local()
{
    static localization local;
    local.figure = "Figure";
    local.listing = "Listing";
    local.table = "Table";
    return local;
}

const char *cDefHead =
"<!DOCTYPE html>\n"
"<head lang=\"en\">\n"
"<meta charset=\"utf-8\">\n"
"<title>md2html</title>\n"
"<style>\n"
"code {\n"
"    background-color: #f8f8f8;\n"
"}\n"
".blk {\n"
"    display: block;\n"
"}\n"
"pre {\n"
"    display: block;\n"
"    width: 100%;\n"
"    background-color: #f8f8f8;\n"
"    padding: 6px 2em 6px 2em;\n"
"    border: 1px solid #ddd;\n"
"}\n"
".p {\n"
"    color: #6f00b1;\n"
"    background-color: #ffffff;\n"
"}\n"
"</style>\n"
"</head>\n";

const char *cStyle =
"<style>\n"
"code {\n"
"    background-color: #f8f8f8;\n"
"}\n"
".blk {\n"
"    display: block;\n"
"}\n"
"pre {\n"
"    display: block;\n"
"    width: 100%;\n"
"    background-color: #f8f8f8;\n"
"    padding: 6px 2em 6px 2em;\n"
"    border: 1px solid #ddd;\n"
"}\n"
".p {\n"
"    color: #6f00b1;\n"
"    background-color: #ffffff;\n"
"}\n"
"</style>\n";

static void
my_rndr_head(hoedown_buffer *ob, metadata * doc_meta, ext_definition * extension)
{
    if (headContents) { // User supplied document header
        hoedown_buffer_puts(ob, headContents);
        return;
    }
    hoedown_buffer_puts(ob, "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n");
    if (doc_meta->title) {
        hoedown_buffer_printf(ob, "<title>%s</title>\n", doc_meta->title);
    }
    if (doc_meta->authors)
    {
        hoedown_buffer_puts(ob, "<meta name=\"author\" content=\"");
        Strings * it;
        for (it = doc_meta->authors; it != NULL; it = (Strings *)it->next)
        {
            if (it->size == 1) {
                hoedown_buffer_puts(ob, it->str);
            }
            else {
                hoedown_buffer_printf(ob, "%s, ", it->str);
            }
        }
        hoedown_buffer_puts(ob, "\">\n");
    }
    if (doc_meta->keywords)
    {
        hoedown_buffer_printf(ob, "<meta name=\"keywords\" content=\"%s\">\n", doc_meta->keywords);
    }
    if (doc_meta->style)
    {
        hoedown_buffer_printf(ob, "<link rel=\"stylesheet\" href=\"%s\">\n", doc_meta->style);
    }
    if (extension && extension->extra_header)
    {
        hoedown_buffer_puts(ob, extension->extra_header);
    }

    hoedown_buffer_puts(ob, "</head>\n<body>\n");
}

/* MAIN LOGIC */

int
main(int argc, char **argv)
{
	struct option_data data;
	clock_t t1, t2;
	FILE *file = stdin;
	hoedown_buffer *ib, *ob;
	hoedown_renderer *renderer = NULL;
	void (*renderer_free)(hoedown_renderer *) = NULL;
	hoedown_document *document;

	/* Parse options */
	data.basename = argv[0];
	data.done = 0;
	data.show_time = 0;
	data.iunit = DEF_IUNIT;
	data.ounit = DEF_OUNIT;
	data.filename = NULL;
	data.renderer = RENDERER_HTML;
	data.toc_level = 0;
	data.render_flags = SCIDOWN_RENDER_CHARTER;
	data.extensions = (hoedown_extensions) (HOEDOWN_EXT_BLOCK | HOEDOWN_EXT_SPAN | HOEDOWN_EXT_FLAGS);
	data.max_nesting = DEF_MAX_NESTING;

	argc = parse_options(argc, argv, &data);
	if (data.done) return 0;
	if (!argc) return 1;

	/* Open input file, if needed */
	if (data.filename) {
		file = fopen(data.filename, "r");
		if (!file) {
			fprintf(stderr, "Unable to open input file \"%s\": %s\n", data.filename, strerror(errno));
			return 5;
		}
	}

	/* Read everything */
	ib = hoedown_buffer_new(data.iunit);

	if (hoedown_buffer_putf(ib, file)) {
		fprintf(stderr, "I/O errors found while reading input.\n");
		return 5;
	}

	if (file != stdin) fclose(file);

	/* Create the renderer */
    if (data.renderer == RENDERER_HTML) {
        renderer = hoedown_html_renderer_new(data.render_flags, data.toc_level, get_local());
        renderer->head = my_rndr_head;
    }
    else if (data.renderer == RENDERER_HTML_TOC) {
        renderer = hoedown_html_toc_renderer_new(data.toc_level, get_local());
    }
    else if (data.renderer == RENDERER_LATEX) {
        renderer = scidown_latex_renderer_new(data.render_flags, data.toc_level, get_local());
    }
	renderer_free = hoedown_html_renderer_free;

	/* Perform Markdown rendering */
	ob = hoedown_buffer_new(data.ounit);

	ext_definition ext = {NULL, NULL};
	if (data.renderer == RENDERER_HTML) {
        ext.extra_header = (char *)cStyle;
		ext.extra_closing = "<!-- rendered by md2html -->\n";
	}
	document = hoedown_document_new(renderer, data.extensions,&ext, NULL, data.max_nesting);

	t1 = clock();
	hoedown_document_render(document, ob, ib->data, ib->size);
	t2 = clock();

	/* Cleanup */
	hoedown_buffer_free(ib);
	hoedown_document_free(document);
	renderer_free(renderer);

	/* Write the result to file or stdout */
    if (OutputFile) {
        FILE *out = fopen(OutputFile, "w");
        if (!out) {
            hoedown_buffer_free(ob);
            fprintf(stderr, "I/O errors unable to create output '%s'.\n", OutputFile);
            return 5;
        }
        size_t wtn = fwrite(ob->data, 1, ob->size, out);
        fclose(out);
        if (wtn != ob->size) {
            hoedown_buffer_free(ob);
            fprintf(stderr, "I/O errors unable to write output '%s'.\n", OutputFile);
            return 5;
        }

    }
    else {
        (void)fwrite(ob->data, 1, ob->size, stdout);
        hoedown_buffer_free(ob);
        if (ferror(stdout)) {
            fprintf(stderr, "I/O errors found while writing output.\n");
            return 5;
        }
    }
    hoedown_buffer_free(ob);

	/* Show rendering time */
	if (data.show_time) {
		double elapsed;

		if (t1 == ((clock_t) -1) || t2 == ((clock_t) -1)) {
			fprintf(stderr, "Failed to get the time.\n");
			return 1;
		}

		elapsed = (double)(t2 - t1) / CLOCKS_PER_SEC;
		if (elapsed < 1)
			fprintf(stderr, "Time spent on rendering: %7.2f ms.\n", elapsed*1e3);
		else
			fprintf(stderr, "Time spent on rendering: %6.3f s.\n", elapsed);
	}
    if (headContents)
        delete headContents;

	return 0;
}
