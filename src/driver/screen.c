#include "src/driver/screen.h"
#include "src/driver/port.h"

// ************* private function declare
int get_offset_row(int offset);
int get_offset_col(int offset);
int get_offset_by_col_row(int col, int row);
int get_offset_by_cursor();
void set_cursor_by_offset(int offset);
int handle_scrolling(int offset);
int print_char_at( char character , int col , int row , char attribute_byte );
void memory_copy(char* source, char* dest, int count);
// ************

int get_offset_row(int offset) { return offset / (2 * MAX_COLS); }
int get_offset_col(int offset) { return (offset - (get_offset_row(offset)*2*MAX_COLS))/2; }

// return offset according to given col and row
int get_offset_by_col_row(int col, int row)
{
    return (row * MAX_COLS + col) * 2;
}

// 
int get_offset_by_cursor()
{
    // The device uses its control register as an index
    // to select its internal registers , of which we are
    // interested in:
    // reg 14: which is the high byte of the cursor ’s offset
    // reg 15: which is the low byte of the cursor ’s offset
    // Once the internal register has been selected , we may read or
    // write a byte on the data register .
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8; /* High byte: << 8 */
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2; /* Position * size of character cell */
}

//
void set_cursor_by_offset(int offset)
{
    offset /= 2; // Convert from cell offset to char offset .

    // This is similar to get_cursor , only now we write
    // bytes to those internal device registers .
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

void memory_copy(char* source, char* dest, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        *(dest + i) = *(source + i);
    }
}

int handle_scrolling(int offset)
{
    int i;

    if (offset < MAX_ROWS * MAX_COLS * 2)
    {
        return offset;
    }

    memory_copy(get_offset_by_col_row(0, 1) + (unsigned char*)VIDEO_ADDRESS, 
        get_offset_by_col_row(0, 0) + (unsigned char*)VIDEO_ADDRESS, 
        MAX_ROWS * (MAX_ROWS - 1) * 2);

    char * last_line = get_offset_by_col_row(0 , MAX_ROWS - 1) + (unsigned char*)VIDEO_ADDRESS ;
    for (i = 0; i < MAX_COLS * 2; i++) {
        last_line[i] = 0;
    }

    offset -= 2 * MAX_ROWS;
    return offset;
}


int print_char_at( char character , int col , int row , char attribute_byte ) {
    /* Create a byte ( char ) pointer to the start of video memory */
    unsigned char * vidmem = ( unsigned char *) VIDEO_ADDRESS;
    int offset ;

    /* If attribute byte is zero , assume the default style . */
    if (!attribute_byte ) {
        attribute_byte = WHITE_ON_BLACK ;
    }
    /* Get the video memory offset for the screen location */
    /* If col and row are non - negative , use them for offset . */
    if ( col >= 0 && row >= 0) {
        offset = get_offset_by_col_row( col , row );
    /* Otherwise , use the current cursor position . */
    } else {
        offset = get_offset_by_cursor();
    }
    // If we see a newline character , set offset to the end of
    // current row , so it will be advanced to the first col
    // of the next row.
    if ( character == '\n') {
        int rows = offset / (2 * MAX_COLS);
        offset = get_offset_by_col_row(79, rows);
    }
    else
    {
        vidmem[offset] = character;
        vidmem[offset+1] = attribute_byte;
    }
    offset += 2;
    offset = handle_scrolling(offset);
    set_cursor_by_offset(offset);
    return offset;
}


// ********** public interface
void clear_screen()
{
    int row = 0;
    int col = 0;
    /* Loop through video memory and write blank characters . */
    for ( row =0; row < MAX_ROWS ; row ++) {
        for ( col =0; col < MAX_COLS ; col ++) {
            print_char_at(' ', col, row, WHITE_ON_BLACK);
        }
    }
    // Move the cursor back to the top left .
    set_cursor_by_offset(get_offset_by_col_row(0, 0));
}

void print_message_at(char *message, int col, int row, char attribute_byte)
{
    int i = 0;
    int offset = 0;

    if (col >= 0 && row >= 0)
    {
        offset = get_offset_by_col_row(col, row);
    }
    else
    {
        offset = get_offset_by_cursor();
    }
    
    row = get_offset_row(offset);
    col = get_offset_col(offset);

    while (message[i] != 0)
    {
        offset = print_char_at(message[i], col, row, attribute_byte);
        i++;
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
}


void print_message(char *message, char attribute_byte)
{
    print_message_at(message, -1, -1, attribute_byte);
}
    

