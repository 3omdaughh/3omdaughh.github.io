# 3omdaughh's Room

## Introduction
Welcome to my Room, this is personal website/blog, hosted on GitHub Pages. The website features a clean, simple and fast loading times.
Check out [The blog-announcement](https://3omdaughh.github.io/Blog-Announcement.html) for more information about the project.

## Goal
- Provide you with insights into my mind
- Allowing you to know more about me
- Understanding my thought process

## Tech Stack
- HTML
- CSS
- C
- Shell

No frameworks or blogging services were used, ensuring instant loading times. A C program (`BlogChapters/c2html.c`) creates blog posts, updates the blog index, and appends an item to the RSS feed (`feed.xml`), while a Shell script automates the uploading process. As a result, the website is fully automated, allowing me to quickly write and publish blogs or update content effortlessly.

The site is dark-mode only, typeset in a self-hosted JetBrains Mono (no external requests, no trackers), with a few lines of optional inline JS on the homepage for the uptime counter — everything degrades gracefully without it.

## Writing a post
1. Put the post body in `BlogChapters/blog.txt`
2. `cd BlogChapters && ./output` (or recompile: `gcc -o output c2html.c`)
3. Enter the title when prompted — the post page, blog index, and `feed.xml` are all updated
4. Run `./dep.sh` from the repo root to publish

## Usage
Feel free to reuse any part of this code for your own projects. The source code is meant to be simple and easy to understand, not optimized for reusability.

## License
This project is licensed under the terms of the GPL-3 license. See the LICENSE file for more information.
