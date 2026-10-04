# File Indexing & Search System

A file indexing and search system implemented in C as part of a Data Structures course.

The application manages files associated with keywords and relevance scores, supporting efficient keyword-based search and ranking operations.

## Features

- Add and remove files
- Add and remove keywords associated with files
- Search files by keyword
- Retrieve the top-k most relevant files for a keyword
- Dynamically update the indexing structure
- Display the current contents of the keyword index

## Data Structures

The project was implemented using several custom data structures:

- **Trie** — used for keyword indexing and search
- **Max-Heap** — used to retrieve the most relevant files for `TOPK` queries
- **Doubly Linked List** — used to store files
- **Linked Lists** — used for keyword lists and references between the Trie and files

## Implemented Operations

- `ADD` — adds a new file and its associated keywords
- `DEL` — removes a file from the system
- `ADDKW` — adds a keyword to an existing file
- `DELKW` — removes a keyword from a file
- `FIND` — searches for files associated with a keyword
- `TOPK` — returns the most relevant files for a keyword
- `PRINT` — displays the contents of the keyword index

## Concepts Practiced

- Dynamic memory allocation
- Pointers
- Linked data structures
- Tree-based indexing
- Priority queues and heaps
- File and keyword management
- Manual memory management in C

## Technologies

- C
- Standard C Library

## Project Structure

```text
file-indexing-search-system/
├── main.c
└── README.md
