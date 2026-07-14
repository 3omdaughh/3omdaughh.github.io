#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define MAX_FILENAME 150

static const char *SITE_URL = "https://3omdaughh.github.io";

void sanitize_filename(char *title, char *filename)
{
	int j = 0;
	for (int i = 0; title[i] != '\0' && j < MAX_FILENAME - 5; i++)
		if (isalnum(title[i]) || title[i] == ' ' || title[i] == '-')
			filename[j++] = (title[i] == ' ') ? '_' : title[i];

	filename[j] = '\0';
	strcat(filename, ".html");
}

/* read whole file into malloc'd buffer, NULL on failure */
static char *slurp(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buf = malloc(size + 1);
	fread(buf, 1, size, f);
	buf[size] = '\0';
	fclose(f);
	return buf;
}

/* insert `insert` right after the first occurrence of `marker` in file */
static int insert_after_marker(const char *path, const char *marker,
                               const char *insert)
{
	char *content = slurp(path);
	if (!content)
	{
		printf("Error opening %s\n", path);
		return 1;
	}

	char *pos = strstr(content, marker);
	if (!pos)
	{
		printf("Error: couldn't find \"%s\" in %s\n", marker, path);
		free(content);
		return 1;
	}
	pos += strlen(marker);

	FILE *out = fopen(path, "w");
	if (!out)
	{
		printf("Error writing to %s\n", path);
		free(content);
		return 1;
	}

	fwrite(content, 1, pos - content, out);
	fputs(insert, out);
	fwrite(pos, 1, strlen(pos), out);
	fclose(out);
	free(content);
	return 0;
}

void update_index_html(const char *filename, const char *title, const char *date)
{
	char row[1024];
	snprintf(row, sizeof(row),
		"\n\t\t\t\t<tr>\n"
		"\t\t\t\t\t<td class=\"title\">\n"
		"\t\t\t\t\t\t<a href=\"%s\">%s</a>\n"
		"\t\t\t\t\t</td>\n"
		"\t\t\t\t\t<td><i>%s</i></td>\n"
		"\t\t\t\t</tr>",
		filename, title, date);

	if (insert_after_marker("index.html", "<tbody>", row) == 0)
		printf("Updated index.html with new blog entry at the top.\n");
}

void update_feed_xml(const char *filename, const char *title,
                     const char *rfc822_date)
{
	char item[2048];
	snprintf(item, sizeof(item),
		"\n<item>\n"
		"<title>%s</title>\n"
		"<link>%s/BlogChapters/%s</link>\n"
		"<guid>%s/BlogChapters/%s</guid>\n"
		"<pubDate>%s</pubDate>\n"
		"<description>%s</description>\n"
		"</item>",
		title, SITE_URL, filename, SITE_URL, filename, rfc822_date, title);

	if (insert_after_marker("../feed.xml", "<!-- posts -->", item) == 0)
		printf("Updated ../feed.xml with new RSS item.\n");
}

int main()
{
	char *title = NULL;
	size_t title_size = 0;
	char filename[MAX_FILENAME];
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	char date[50], rfc822[64];
	strftime(date, sizeof(date), "%a, %b %d.%Y", tm_info);
	strftime(rfc822, sizeof(rfc822), "%a, %d %b %Y %H:%M:%S %z", tm_info);

	printf("Enter Blog title: ");
	getline(&title, &title_size, stdin);
	title[strcspn(title, "\n")] = 0;
	sanitize_filename(title, filename);

	char *text = slurp("blog.txt");
	if (!text)
	{
		printf("Error: Could not open blog.txt\n");
		free(title);
		return 1;
	}

	FILE *htmlFile = fopen(filename, "w");
	if (!htmlFile)
	{
		printf("Error opening file for writing.\n");
		free(title);
		free(text);
		return 1;
	}

	fprintf(htmlFile,
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"\t<meta charset=\"UTF-8\">\n"
		"\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"\t<title>%s — 3omdaughh</title>\n"
		"\t<link rel=\"stylesheet\" href=\"style.css\">\n"
		"\t<link rel=\"alternate\" type=\"application/rss+xml\" title=\"3omdaughh's Room\" href=\"../feed.xml\">\n"
		"\t<link rel=\"icon\" href=\"data:image/svg+xml,%%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 16 16'%%3E%%3Crect width='16' height='16' fill='%%230c0c0c'/%%3E%%3Ctext x='8' y='12' font-family='monospace' font-size='11' fill='%%233d9bff' text-anchor='middle'%%3E3%%3C/text%%3E%%3C/svg%%3E\">\n"
		"</head>\n"
		"<body>\n"
		"<div class=\"layout\">\n\n"
		"\t<aside class=\"side\">\n"
		"\t\t<div class=\"side-inner\">\n"
		"\t\t\t<a class=\"brand\" href=\"../index.html\">3omdaughh.log</a>\n"
		"\t\t\t<div class=\"dashes\">--</div>\n"
		"\t\t\t<nav>\n"
		"\t\t\t\t<a href=\"../index.html\">home</a>\n"
		"\t\t\t\t<a href=\"../AboutMe.html\">about</a>\n"
		"\t\t\t\t<a href=\"index.html\" class=\"active\">blog</a>\n"
		"\t\t\t\t<a href=\"../softwareIuse/index.html\">uses</a>\n"
		"\t\t\t\t<a href=\"../Contact.html\">contact</a>\n"
		"\t\t\t</nav>\n"
		"\t\t\t<div class=\"side-foot\">\n"
		"\t\t\t\t<a href=\"../feed.xml\">RSS</a> /\n"
		"\t\t\t\t<a href=\"https://github.com/3omdaughh/3omdaughh.github.io\">src</a>\n"
		"\t\t\t</div>\n"
		"\t\t</div>\n"
		"\t</aside>\n\n"
		"\t<main>\n"
		"\t\t<article>\n"
		"\t\t\t<h1>%s</h1>\n"
		"\t\t\t<p class=\"date\">%s</p>\n"
		"\t\t\t<div class=\"post-body\">\n",
		title, title, date);

	/* write text content with <br> formatting */
	for (char *p = text; *p; p++)
	{
		if (*p == '\n') fprintf(htmlFile, "<br>");
		else fputc(*p, htmlFile);
	}

	fprintf(htmlFile,
		"<br><br><span style=\"color:#808080\"><i>Finished at %s</i></span>\n"
		"\t\t\t</div>\n"
		"\t\t</article>\n\n"
		"\t\t<footer class=\"foot\">\n"
		"\t\t\t<a href=\"index.html\">&larr; All posts</a>\n"
		"\t\t</footer>\n"
		"\t</main>\n\n"
		"</div>\n"
		"</body>\n"
		"</html>\n",
		date);
	fclose(htmlFile);

	printf("Blog post saved as: %s\n", filename);
	update_index_html(filename, title, date);
	update_feed_xml(filename, title, rfc822);

	free(title);
	free(text);
	return 0;
}
