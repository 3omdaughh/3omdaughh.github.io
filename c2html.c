#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define MAX_FILENAME 150
#define MAX_HOME_POSTS 5
#define HOME_INDEX "index.html"
#define SITE_FEED "feed.xml"

static const char *SITE_URL = "https://3omdaughh.github.io";

/* every post belongs to one section; run as `./output <name>` from the repo root */
struct section {
	const char *name;        /* argv[1], also the nav label */
	const char *dir;         /* posts, index.html and blog.txt live here */
	const char *home_marker; /* list on the homepage that gets the newest posts */
	const char *feed;        /* section-only feed (relative to dir), NULL if none */
};

static const struct section sections[] = {
	{ "tech", "tech",         "<!-- latest-tech -->", "feed.xml" },
	{ "log",  "BlogChapters", "<!-- latest-log -->",  NULL },
};
#define NSECTIONS (sizeof(sections) / sizeof(sections[0]))

/* sidebar nav, as seen from one directory below the root */
static const struct { const char *label, *href; } nav[] = {
	{ "home",    "../index.html" },
	{ "about",   "../AboutMe.html" },
	{ "tech",    "../tech/index.html" },
	{ "log",     "../BlogChapters/index.html" },
	{ "uses",    "../softwareIuse/index.html" },
	{ "contact", "../Contact.html" },
};

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

void update_index_html(const struct section *sec, const char *filename,
                       const char *title, const char *date)
{
	char row[1024], path[256];
	snprintf(row, sizeof(row),
		"\n\t\t\t\t<tr>\n"
		"\t\t\t\t\t<td class=\"title\">\n"
		"\t\t\t\t\t\t<a href=\"%s\">%s</a>\n"
		"\t\t\t\t\t</td>\n"
		"\t\t\t\t\t<td><i>%s</i></td>\n"
		"\t\t\t\t</tr>",
		filename, title, date);

	snprintf(path, sizeof(path), "%s/index.html", sec->dir);
	if (insert_after_marker(path, "<tbody>", row) == 0)
		printf("Updated %s with new entry at the top.\n", path);
}

/* keep at most `max` <li> items in the list that follows `marker` */
static int trim_list(const char *path, const char *marker, int max)
{
	char *content = slurp(path);
	if (!content)
	{
		printf("Error opening %s\n", path);
		return 1;
	}

	char *pos = strstr(content, marker);
	char *end = pos ? strstr(pos, "</ul>") : NULL;
	if (!end)
	{
		free(content);
		return 1;
	}
	pos += strlen(marker);

	/* walk to the first <li> past the limit; nothing to do if it isn't there */
	char *li = pos;
	for (int n = 0; n <= max; n++)
	{
		li = strstr(li, "<li>");
		if (!li || li >= end)
		{
			free(content);
			return 0;
		}
		if (n < max)
			li += 4;
	}

	/* drop the leading whitespace of the cut item and keep the one before </ul> */
	char *cut = li;
	while (cut > pos && isspace((unsigned char)cut[-1]))
		cut--;

	char *tail = end;
	while (tail > pos && isspace((unsigned char)tail[-1]))
		tail--;

	FILE *out = fopen(path, "w");
	if (!out)
	{
		printf("Error writing to %s\n", path);
		free(content);
		return 1;
	}

	fwrite(content, 1, cut - content, out);
	fputs(tail, out);
	fclose(out);
	free(content);
	return 0;
}

void update_home_index(const struct section *sec, const char *filename,
                       const char *title, const char *iso_date)
{
	char row[1024];
	snprintf(row, sizeof(row),
		"\n\t\t\t\t\t\t<li><a href=\"%s/%s\">%s</a>"
		"<time>%s</time></li>",
		sec->dir, filename, title, iso_date);

	if (insert_after_marker(HOME_INDEX, sec->home_marker, row) == 0)
	{
		trim_list(HOME_INDEX, sec->home_marker, MAX_HOME_POSTS);
		printf("Updated %s with new entry at the top.\n", HOME_INDEX);
	}
}

void update_feed_xml(const char *feed, const struct section *sec,
                     const char *filename, const char *title,
                     const char *rfc822_date)
{
	char item[2048];
	snprintf(item, sizeof(item),
		"\n<item>\n"
		"<title>%s</title>\n"
		"<link>%s/%s/%s</link>\n"
		"<guid>%s/%s/%s</guid>\n"
		"<pubDate>%s</pubDate>\n"
		"<description>%s</description>\n"
		"</item>",
		title, SITE_URL, sec->dir, filename, SITE_URL, sec->dir, filename,
		rfc822_date, title);

	if (insert_after_marker(feed, "<!-- posts -->", item) == 0)
		printf("Updated %s with new RSS item.\n", feed);
}

static void write_escaped(FILE *out, const char *s, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		switch (s[i])
		{
		case '<': fputs("&lt;", out); break;
		case '>': fputs("&gt;", out); break;
		case '&': fputs("&amp;", out); break;
		default:  fputc(s[i], out);
		}
	}
}

/* blog.txt -> post body. Plain lines are raw HTML joined with <br>,
 * `inline code` is escaped into <code>, and lines between ``` fences
 * become an escaped <pre><code> block (```lang adds a language class). */
static void write_body(FILE *out, const char *text)
{
	int in_code = 0, first_code_line = 0;
	const char *p = text;

	while (*p)
	{
		const char *eol = strchr(p, '\n');
		size_t len = eol ? (size_t)(eol - p) : strlen(p);

		if (len >= 3 && strncmp(p, "```", 3) == 0)
		{
			if (!in_code)
			{
				size_t lang = len - 3;
				while (lang && isspace((unsigned char)p[3 + lang - 1]))
					lang--;
				if (lang)
				{
					fputs("<pre><code class=\"language-", out);
					write_escaped(out, p + 3, lang);
					fputs("\">", out);
				}
				else
					fputs("<pre><code>", out);
				first_code_line = 1;
			}
			else
				fputs("</code></pre>", out);
			in_code = !in_code;
		}
		else if (in_code)
		{
			if (!first_code_line)
				fputc('\n', out);
			first_code_line = 0;
			write_escaped(out, p, len);
		}
		else
		{
			for (size_t i = 0; i < len; i++)
			{
				const char *close = NULL;
				if (p[i] == '`')
					close = memchr(p + i + 1, '`', len - i - 1);
				if (close)
				{
					fputs("<code>", out);
					write_escaped(out, p + i + 1, close - (p + i + 1));
					fputs("</code>", out);
					i = close - p;
				}
				else
					fputc(p[i], out);
			}
			if (eol)
				fputs("<br>", out);
		}
		p = eol ? eol + 1 : p + len;
	}

	if (in_code)
		fputs("</code></pre>", out);
}

static void usage(void)
{
	fputs("usage: ./output <section>\n  sections:", stderr);
	for (size_t i = 0; i < NSECTIONS; i++)
		fprintf(stderr, " %s", sections[i].name);
	fputc('\n', stderr);
}

int main(int argc, char **argv)
{
	const struct section *sec = NULL;
	if (argc == 2)
		for (size_t i = 0; i < NSECTIONS; i++)
			if (strcmp(argv[1], sections[i].name) == 0)
				sec = &sections[i];
	if (!sec)
	{
		usage();
		return 1;
	}

	char *title = NULL;
	size_t title_size = 0;
	char filename[MAX_FILENAME], path[256];
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	char date[50], rfc822[64], iso[16];
	strftime(date, sizeof(date), "%a, %b %d.%Y", tm_info);
	strftime(iso, sizeof(iso), "%Y-%m-%d", tm_info);
	strftime(rfc822, sizeof(rfc822), "%a, %d %b %Y %H:%M:%S %z", tm_info);

	printf("Enter %s post title: ", sec->name);
	if (getline(&title, &title_size, stdin) < 0)
	{
		free(title);
		return 1;
	}
	title[strcspn(title, "\n")] = 0;
	sanitize_filename(title, filename);

	snprintf(path, sizeof(path), "%s/blog.txt", sec->dir);
	char *text = slurp(path);
	if (!text)
	{
		printf("Error: Could not open %s\n", path);
		free(title);
		return 1;
	}

	snprintf(path, sizeof(path), "%s/%s", sec->dir, filename);
	FILE *htmlFile = fopen(path, "w");
	if (!htmlFile)
	{
		printf("Error opening %s for writing.\n", path);
		free(title);
		free(text);
		return 1;
	}

	const char *feed_href = sec->feed ? sec->feed : "../" SITE_FEED;

	fprintf(htmlFile,
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"\t<meta charset=\"UTF-8\">\n"
		"\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"\t<title>%s — 3omdaughh</title>\n"
		"\t<link rel=\"stylesheet\" href=\"style.css\">\n"
		"\t<link rel=\"alternate\" type=\"application/rss+xml\" title=\"3omdaughh's Room\" href=\"%s\">\n"
		"\t<link rel=\"icon\" type=\"image/png\" href=\"../favicon.png\">\n"
		"</head>\n"
		"<body>\n"
		"<div class=\"layout\">\n\n"
		"\t<aside class=\"side\">\n"
		"\t\t<div class=\"side-inner\">\n"
		"\t\t\t<a class=\"brand\" href=\"../index.html\">3omdaughh.log</a>\n"
		"\t\t\t<div class=\"dashes\">--</div>\n"
		"\t\t\t<nav>\n",
		title, feed_href);

	for (size_t i = 0; i < sizeof(nav) / sizeof(nav[0]); i++)
	{
		if (strcmp(nav[i].label, sec->name) == 0)
			fprintf(htmlFile, "\t\t\t\t<a href=\"index.html\" class=\"active\">%s</a>\n",
				nav[i].label);
		else
			fprintf(htmlFile, "\t\t\t\t<a href=\"%s\">%s</a>\n",
				nav[i].href, nav[i].label);
	}

	fprintf(htmlFile,
		"\t\t\t</nav>\n"
		"\t\t\t<div class=\"side-foot\">\n"
		"\t\t\t\t<a href=\"%s\">RSS</a> /\n"
		"\t\t\t\t<a href=\"https://github.com/3omdaughh/3omdaughh.github.io\">src</a>\n"
		"\t\t\t</div>\n"
		"\t\t</div>\n"
		"\t</aside>\n\n"
		"\t<main>\n"
		"\t\t<article>\n"
		"\t\t\t<h1>%s</h1>\n"
		"\t\t\t<p class=\"date\">%s</p>\n"
		"\t\t\t<div class=\"post-body\">\n",
		feed_href, title, date);

	write_body(htmlFile, text);

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

	printf("Post saved as: %s\n", path);
	update_index_html(sec, filename, title, date);
	update_home_index(sec, filename, title, iso);
	update_feed_xml(SITE_FEED, sec, filename, title, rfc822);
	if (sec->feed)
	{
		snprintf(path, sizeof(path), "%s/%s", sec->dir, sec->feed);
		update_feed_xml(path, sec, filename, title, rfc822);
	}

	free(title);
	free(text);
	return 0;
}
