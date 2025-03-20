#!/bin/sh

# Get the commit description
read -p "Commit: " Commit_des

git add --all
git commit -m "$Commit_des"
git push -u origin main
