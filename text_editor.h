#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H



typedef struct {
  int start;
  int length;

  char *source;
} Pieces;

typedef struct {
  Pieces data;

  Pieces *next;
  Pieces *ant;
} PieceCell;


typedef struct {
  PieceCell *vf;
} PieceList;

typedef struct TTextEditor {
  char *original_buffer;
  char *add_buffer;
  
  int add_buffer_size;
  int add_buffer_capacity;

  int cursor_position;
  int total_length;

  PieceList pieces;
} TextEditor;


// Create a new text editor
// filename: if not NULL, load content from this file
// text: if not NULL (and filename is NULL), initialize with this text
// global_cursor: initial cursor position
TextEditor *create_table(const char *filename, const char *text, int global_cursor);

// Move cursor by 'advance' positions (can be negative for backwards)
void advance_cursor(TextEditor *editor, int advance);

// Get current cursor position
int show_global_cursor(TextEditor *editor);

// Get total text length
int show_total_len(TextEditor *editor);

// Insert text at current cursor position
void add_text(TextEditor *editor, char *text);

// Delete 'length' characters before cursor
void delete_text(TextEditor *editor, int length);

// Extract full text as allocated string (caller must free)
char *extract_current_text(TextEditor *editor);

// Save text to file or return via text pointer
void save_current_text(TextEditor *editor, const char *filename, char **text, int *global_cursor);

// ============================================================================
// Helper functions (optional - you can define your own)
// ============================================================================

void list_init(PieceList *list);

PieceCell* create_piece_cell(int start, int length, char *source);

void append_piece(PieceList *list, int start, int length, char *source);

// adaugare in interiorul listei
void insert_piece(PieceList *list, PieceCell *prev_node, PieceCell *node);

void delete_piece(PieceList *list, PieceCell *node);

#endif // TEXT_EDITOR_H
