#include <stdio.h>

int main() 
{
    char text[1000];
    char title[25];

    // Get text input from the user
    printf("Enter Blog title: ");
    fgets(title, sizeof(title), stdin);
    printf("Enter text:\n");
    fgets(text, sizeof(text), stdin);

    // Open an HTML file for writing
    FILE *htmlFile = fopen("newblog.html", "w");

    // Check if the file is successfully opened
    if (htmlFile == NULL) 
    {
        printf("Error opening file for writing.\n");
        return 1;
    }

    // Write HTML headers and the entered text to the file
    fprintf(htmlFile, "<!DOCTYPE html>\n");
    fprintf(htmlFile, "<html>\n");
    fprintf(htmlFile, "<head>\n");
    fprintf(htmlFile, "<meta charset=\"UTF-8\">\n");
    fprintf(htmlFile, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(htmlFile, "<link rel=\"stylesheet\" href=\"style.css\">\n");
    fprintf(htmlFile, "<title>%s</title>\n", title);
    fprintf(htmlFile, "</head>\n");
    fprintf(htmlFile, "<body>\n");
    fprintf(htmlFile, "<div class=\"center-text\">\n");
    fprintf(htmlFile, "<nav>\n");
    fprintf(htmlFile, "<a href=\"../index.html\">Home</a>\n");
    fprintf(htmlFile, "<a href=\"../AboutMe.html\">About me</a>\n");
    fprintf(htmlFile, "<a href=\"index.html\">Blog</a>\n");
    fprintf(htmlFile, "<a href=\"../WIDU/index.html\">WIDU</a>\n");
    fprintf(htmlFile, "<a href=\"../GIT.html\">Get in Touch</a>\n");
    fprintf(htmlFile, "<a href=\"../RSS.html\">RSS</a>\n");
    fprintf(htmlFile, "</nav>\n");
    fprintf(htmlFile, "</div>\n");
    fprintf(htmlFile, "<div class=\"container\">\n");
    fprintf(htmlFile, "<h1>%s</h1>\n", title);
    fprintf(htmlFile, "</div>\n");
    fprintf(htmlFile, "<div class=\"bar\"></div>\n");
    fprintf(htmlFile, "<div class=\"mainContainer\" style=\"margin:2%;background-color:#1c1c1c; padding:5%;\">\n");
    fprintf(htmlFile, "<p>%s</p>\n", text);
    fprintf(htmlFile, "</div>\n");
    fprintf(htmlFile, "</div>\n");
    fprintf(htmlFile, "</body>\n");
    fprintf(htmlFile, "</html>\n");

    // Close the HTML file
    fclose(htmlFile);

    printf("HTML file created successfully");

    return 0;
}

