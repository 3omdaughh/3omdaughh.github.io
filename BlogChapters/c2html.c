#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define MAX_FILENAME 150

void sanitize_filename(char *title, char *filename) 
{
	int j = 0;
	for (int i = 0; title[i] != '\0' && j < MAX_FILENAME - 5; i++) 
		if (isalnum(title[i]) || title[i] == ' ' || title[i] == '-') 
			filename[j++] = (title[i] == ' ') ? '_' : title[i];

	filename[j] = '\0';
	strcat(filename, ".html");
}

void update_index_html(const char *filename, const char *title, const char *date) 
{
	FILE *indexFile = fopen("index.html", "r");
	if (!indexFile) 
	{
		printf("Error opening index.html\n");
		return;
	}

	fseek(indexFile, 0, SEEK_END);
	long file_size = ftell(indexFile);
	fseek(indexFile, 0, SEEK_SET);

	char *htmlContent = malloc(file_size + 1);
	fread(htmlContent, 1, file_size, indexFile);
	htmlContent[file_size] = '\0';
	fclose(indexFile);

	char *pos = strstr(htmlContent, "<tbody>");
	if (!pos) 
	{
		printf("Error: Couldn't find <tbody> in index.html\n");
		free(htmlContent);
		return;
	}
	pos += 7;

	FILE *indexFileNew = fopen("index.html", "w");
	if (!indexFileNew) 
	{
		printf("Error writing to index.html\n");
		free(htmlContent);
		return;
	}

	fwrite(htmlContent, 1, pos - htmlContent, indexFileNew);
	fprintf(indexFileNew,
		"\t\t\t\t<tr>\n"
		"\t\t\t\t\t<td class=\"title\">\n"
		"\t\t\t\t\t\t<a href=\"%s\">%s</a>\n"
		"\t\t\t\t\t</td>\n"
		"\t\t\t\t\t<td><i>%s</i></td>\n"
		"\t\t\t\t</tr>\n",
		filename, title, date);
	fwrite(pos, 1, strlen(pos), indexFileNew);
	fclose(indexFileNew);
	free(htmlContent);
	printf("Updated index.html with new blog entry at the top.\n");
}

int main() 
{
	char *title = NULL;
	size_t title_size = 0;
	char filename[MAX_FILENAME];
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	char date[50];
	strftime(date, sizeof(date), "%a, %b %d.%Y", tm_info);

	printf("Enter Blog title: ");
	getline(&title, &title_size, stdin);
	title[strcspn(title, "\n")] = 0;
	sanitize_filename(title, filename);

	FILE *file = fopen("blog.txt", "r");
	if (!file)
	{
		printf("Error: Could not open blog.txt\n");
		free(title);
		return 1;
	}

	fseek(file, 0, SEEK_END);
	long text_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *text = malloc(text_size + 1);
	fread(text, 1, text_size, file);
	text[text_size] = '\0';
	fclose(file);

	FILE *htmlFile = fopen(filename, "w");
	if (!htmlFile) 
	{
	    printf("Error opening file for writing.\n");
	    free(title);
	    free(text);
	    return 1;
	}

	fprintf(htmlFile, "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n");
	fprintf(htmlFile, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");

	fprintf(htmlFile, "<div class=\"center-text\">\n<nav>\n");
	fprintf(htmlFile, "<a href=\"../AboutMe.html\">About me</a>\n");
	fprintf(htmlFile, "<a href=\"index.html\">Blog</a>\n");
	fprintf(htmlFile, "<a href=\"../Philosophy.html\">Philosophy</a>\n");
	fprintf(htmlFile, "<a href=\"../Contact.html\">Contact</a>\n");
	//fprintf(htmlFile, "<a href=\"../RSS.html\">RSS</a>\n");
	fprintf(htmlFile, "</nav>\n</div>\n");
	fprintf(htmlFile, "<link rel=\"stylesheet\" href=\"style.css\">\n<title>%s</title>\n</head>\n", title);
	fprintf(htmlFile, "<body>\n<div class=\"container\">\n<h1>%s</h1>\n", title);
	fprintf(htmlFile, "</div>\n");

	fprintf(htmlFile, "<div class=\"bar\"></div>\n");
	fprintf(htmlFile, "<div class=\"mainContainer\" style=\"margin:2%%;background-color:#1c1c1c; padding:5%%;\">\n");

	// Write text content with <br> formatting
	for (char *p = text; *p; p++) 
	{
		if (*p == '\n') fprintf(htmlFile, "<br>");
		else fputc(*p, htmlFile);
	}

	fprintf(htmlFile, "<br><br><span style=\"color:#808080\"><i>Finished at %s</i></span>\n", date);
	fprintf(htmlFile, "</div>\n</body>\n</html>\n<div class=\"bar\"></div>\n");
	fclose(htmlFile);

	printf("Blog post saved as: %s\n", filename);
	update_index_html(filename, title, date);

	free(title);
	free(text);
	return 0;
}
