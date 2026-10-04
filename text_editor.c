#include "text_editor.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void list_init(PieceList *list) {
	if (list)
		list->vf = NULL;
}

PieceCell* create_piece_cell(int start, int length, char *source) {
	PieceCell *piece = malloc(sizeof(PieceCell));

	if (!piece)
		return NULL;
	piece->data.start = start;
	piece->data.length = length;
	piece->data.source = source;
	piece->ant = NULL;
	piece->next = NULL;

	return piece;
}

void insert_piece(PieceList *list, PieceCell *prev_node, PieceCell *node) {
	if (!node)
		return;
	if (prev_node == NULL) {
		node->next = list->vf;
		node->ant = NULL;
		if (list->vf != NULL)
			list->vf->ant = node;
		list->vf = node;
	} else {
		node->next = prev_node->next;
		node->ant = prev_node;
		if (prev_node->next != NULL)
			((PieceCell*)prev_node->next)->ant = node;
		prev_node->next = node;
	}
}

void delete_piece(PieceList *list, PieceCell *node) {
	if (!list || !node)
		return;
	if (list->vf == node)
		list->vf = node->next;
	if (node->ant)
		((PieceCell*)node->ant)->next = node->next;
	if (node->next)
		((PieceCell*)node->next)->ant = node->ant;
	free(node);
}

TextEditor *create_table(const char *filename, const char *text, int global_cursor) {
	TextEditor *table = calloc(1, sizeof(TextEditor));
	if (!table)
		return NULL;

	list_init(&table->pieces);
	table->add_buffer_capacity = 1024 * 1024;	// pt eficienta si in lipsa restrictiilor de memorie am ales dimensiunea initiala de 1mb
	table->add_buffer = malloc(table->add_buffer_capacity);
	table->add_buffer_size = 0;

	if (filename) {
		struct stat st;
		if (stat(filename, &st) == 0) {
			table->original_buffer = malloc(st.st_size + 1);
			FILE *f = fopen(filename, "r");
			if (f) {
				fread(table->original_buffer, 1, st.st_size, f);
				table->original_buffer[st.st_size] = '\0';
				fclose(f);
				PieceCell *initial = create_piece_cell(0, st.st_size, table->original_buffer);
				insert_piece(&table->pieces, NULL, initial);
				table->total_length = st.st_size;
			}
		}
	} else if (text) {
		table->original_buffer = strdup(text);
		int len = strlen(table->original_buffer);
		PieceCell *initial = create_piece_cell(0, len, table->original_buffer);
		insert_piece(&table->pieces, NULL, initial);
		table->total_length = len;
	}
	
	table->cursor_position = global_cursor;
	return table;
}

void add_text(TextEditor *editor, char *text) {
	int len = strlen(text);
	if (len == 0)
		return;

	if (editor->add_buffer_size + len >= editor->add_buffer_capacity) {
		while (editor->add_buffer_size + len >= editor->add_buffer_capacity)
			editor->add_buffer_capacity *= 2;
		editor->add_buffer = realloc(editor->add_buffer, editor->add_buffer_capacity);
	}

	int start_in_buffer = editor->add_buffer_size;
	memcpy(editor->add_buffer + start_in_buffer, text, len);
	editor->add_buffer_size += len;

	PieceCell *curr = editor->pieces.vf;
	int offset = 0;
	while (curr && offset + curr->data.length < editor->cursor_position) {
		offset += curr->data.length;
		curr = curr->next;
	}

	if (curr && (offset + curr->data.length == editor->cursor_position) && 
		curr->data.source == editor->add_buffer &&
		(curr->data.start + curr->data.length == start_in_buffer)) {
		curr->data.length += len;
	} else {
		PieceCell *new_p = create_piece_cell(start_in_buffer, len, editor->add_buffer);
		if (!curr) {
			PieceCell *last = editor->pieces.vf;
			while (last && last->next)
				last = last->next;
			insert_piece(&editor->pieces, last, new_p);
		} else {
			int split_idx = editor->cursor_position - offset;
			if (split_idx == 0) {
				insert_piece(&editor->pieces, curr->ant, new_p);
			} else if (split_idx == curr->data.length) {
				insert_piece(&editor->pieces, curr, new_p);
			} else {
				int right_len = curr->data.length - split_idx;
				PieceCell *right_p = create_piece_cell(curr->data.start + split_idx, right_len, curr->data.source);
				curr->data.length = split_idx;
				insert_piece(&editor->pieces, curr, right_p);
				insert_piece(&editor->pieces, curr, new_p);
			}
		}
	}

	editor->total_length += len;
	editor->cursor_position += len;
}

void delete_text(TextEditor *editor, int length) {
	if (length <= 0 || editor->cursor_position == 0)
		return;
	if (length > editor->cursor_position)
		length = editor->cursor_position;

	int remaining = length;
	int target_start = editor->cursor_position - length;
	editor->cursor_position = target_start;
	editor->total_length -= length;

	PieceCell *curr = editor->pieces.vf;
	int offset = 0;
	while (curr && offset + curr->data.length <= target_start) {
		offset += curr->data.length;
		curr = curr->next;
	}

	while (curr && remaining > 0) {
		int pos_in_piece = target_start - offset;
		if (pos_in_piece < 0) pos_in_piece = 0;

		int available = curr->data.length - pos_in_piece;
		int to_delete = (available > remaining) ? remaining : available;

		if (pos_in_piece == 0 && to_delete == curr->data.length) {
			PieceCell *next = curr->next;
			delete_piece(&editor->pieces, curr);
			curr = next;
		} else if (pos_in_piece == 0) {
			curr->data.start += to_delete;
			curr->data.length -= to_delete;
			curr = curr->next;
		} else if (pos_in_piece + to_delete == curr->data.length) {
			curr->data.length -= to_delete;
			curr = curr->next;
		} else {
			int right_start = curr->data.start + pos_in_piece + to_delete;
			int right_len = curr->data.length - (pos_in_piece + to_delete);
			PieceCell *right_p = create_piece_cell(right_start, right_len, curr->data.source);
			curr->data.length = pos_in_piece;
			insert_piece(&editor->pieces, curr, right_p);
			curr = right_p->next;
		}
		remaining -= to_delete;
		offset += (pos_in_piece + to_delete);
	}
}

char *extract_current_text(TextEditor *editor) {
	char *res = calloc(1, editor->total_length + 1);
	if (!res)
		return NULL;
	char *p = res;
	PieceCell *curr = editor->pieces.vf;
	while (curr) {
		memcpy(p, curr->data.source + curr->data.start, curr->data.length);
		p += curr->data.length;
		curr = curr->next;
	}
	*p = '\0';
	return res;
}

void advance_cursor(TextEditor *editor, int advance) {
	editor->cursor_position += advance;
	if (editor->cursor_position < 0)
		editor->cursor_position = 0;
	if (editor->cursor_position > editor->total_length)
		editor->cursor_position = editor->total_length;
}

int show_global_cursor(TextEditor *editor)	{
	return editor->cursor_position;
}

int show_total_len(TextEditor *editor) {
	return editor->total_length;
}

void save_current_text(TextEditor *editor, const char *filename, char **text, int *global_cursor) {
	char *full_text = extract_current_text(editor);
	if (filename) {
		FILE *f = fopen(filename, "w");
		if (f) {
			fwrite(full_text, 1, editor->total_length, f);
			fclose(f);
		}
	}
	if (text) 
		*text = full_text;
	else
		free(full_text);
	if (global_cursor)
		*global_cursor = editor->cursor_position;
}