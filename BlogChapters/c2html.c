#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define MAX_TITLE 100
#define MAX_TEXT 1000
#define MAX_FILENAME 150
#define MAX_HTML 10000

// Function to sanitize the title and generate a valid filename
void sanitize_filename(char *title, char *filename) {
    int j = 0;
    for (int i = 0; title[i] != '\0' && j < MAX_FILENAME - 5; i++) {
        if (isalnum(title[i]) || title[i] == ' ' || title[i] == '-') {
            filename[j++] = (title[i] == ' ') ? '_' : title[i];  // Replace spaces with underscores
        }
    }
    filename[j] = '\0';
    strcat(filename, ".html");  // Append file extension
}

// Function to update index.html by adding the new blog entry at the top
void update_index_html(const char *filename, const char *title, const char *date) {
    FILE *indexFile = fopen("index.html", "r");
    if (indexFile == NULL) {
        printf("Error opening index.html\n");
        return;
    }

    // Read the entire index.html content into a buffer
    char htmlContent[MAX_HTML];
    size_t len = fread(htmlContent, 1, sizeof(htmlContent) - 1, indexFile);
    htmlContent[len] = '\0';
    fclose(indexFile);

    // Find the start of the <tbody> section
    char *pos = strstr(htmlContent, "<tbody>");
    if (!pos) {
        printf("Error: Couldn't find <tbody> in index.html\n");
        return;
    }
    pos += 7; // Move past "<tbody>"

    // Prepare the new blog entry
    char newEntry[300];
    snprintf(newEntry, sizeof(newEntry),
             "\t\t\t\t<tr>\n"
             "\t\t\t\t\t<td class=\"title\">\n"
             "\t\t\t\t\t\t<a href=\"%s\">%s</a>\n"
             "\t\t\t\t\t</td>\n"
             "\t\t\t\t\t<td>\n"
             "\t\t\t\t\t\t<i>%s</i>\n"
             "\t\t\t\t\t</td>\n"
             "\t\t\t\t</tr>\n",
             filename, title, date);

    // Create the updated index content
    FILE *indexFileNew = fopen("index.html", "w");
    if (indexFileNew == NULL) {
        printf("Error writing to index.html\n");
        return;
    }

    // Write everything up to `<tbody>`
    fwrite(htmlContent, 1, pos - htmlContent, indexFileNew);
    // Write the new blog entry at the top
    fwrite(newEntry, 1, strlen(newEntry), indexFileNew);
    // Write the remaining content
    fwrite(pos, 1, strlen(pos), indexFileNew);

    fclose(indexFileNew);
    printf("Updated index.html with new blog entry at the top.\n");
}

int main() {
    char title[MAX_TITLE];
    char text[MAX_TEXT];
    char filename[MAX_FILENAME];

    time_t t;
    struct tm *tm_info;
    char date[50];

    // Get the current time
    time(&t);
    tm_info = localtime(&t);
    strftime(date, sizeof(date), "%a, %b %d.%Y", tm_info);  // Format: Sun, Mar 17.2025

    // Get title input from the user
    printf("Enter Blog title: ");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0; // Remove newline

    // Sanitize title and create filename
    sanitize_filename(title, filename);

    // Get text input from the user
    printf("Enter text:\n");
    fgets(text, sizeof(text), stdin);
    text[strcspn(text, "\n")] = 0; // Remove newline

    // Open the HTML file with the dynamic filename
    FILE *htmlFile = fopen(filename, "w");
    if (htmlFile == NULL) {
        printf("Error opening file for writing.\n");
        return 1;
    }

    // Write HTML headers and the entered text to the file
    fprintf(htmlFile, "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n");
    fprintf(htmlFile, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");

    fprintf(htmlFile, "<div class=\"center-text\">\n<nav>\n");
    fprintf(htmlFile, "<a href=\"../index.html\">Home</a>\n");
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
    for (char *p = text; *p; p++) {
        if (*p == '\n') 
            fprintf(htmlFile, "<br>");
        else 
            fputc(*p, htmlFile);
    }

    fprintf(htmlFile, "<br><br><span style=\"color:#808080\"><i>Finished at %s</i></span>\n", date);
    fprintf(htmlFile, "</div>\n</body>\n</html>\n");
    fprintf(htmlFile, "<div class=\"bar\"></div>\n");

    fclose(htmlFile);
    printf("Blog post saved as: %s\n", filename);

    // Update index.html with the new blog entry
    update_index_html(filename, title, date);

    return 0;
}
