#ifndef LIST_FILES_H
#define LIST_FILES_H

void list_config_files(const char *dir_path, const char *file_extension);

#endif // LIST_FILES_H

void read_config_file(const char *file_path);

void search_and_read_config_files(const char *dir_path, const char *file_extension);

int contains_keyword(const char *line);