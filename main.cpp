#include <iostream>
#include <string>
#include <fstream>
#include <stack>
#include <set>
#include <chrono>

using namespace std;

int SIZE, SIZE_ROOT, SIZE_SQUARE;
stack <int> answer;

chrono::steady_clock::time_point start_t;


int int_sqrt(int x) {
    for (int i = 0; i < x; i++) {
        if (i * i == x)
            return i;
    }
    return 0;
}

void read_input(string filename, int *&table) {
    ifstream fin(filename);

    fin >> SIZE;
    SIZE_ROOT = int_sqrt(SIZE);
    SIZE_SQUARE = SIZE * SIZE;

    table = new int[SIZE_SQUARE];
    for (int i = 0; i < SIZE_SQUARE; i++) {
        fin >> table[i];
    }
    fin.close();
}

void print_table(int *table) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            cout << table[i * SIZE + j] << " ";
        }
        cout << endl;
    }
}

struct LinkBase
{
    LinkBase *above;
    LinkBase *below;
    LinkBase *left;
    LinkBase *right;
    LinkBase *column;

    virtual bool is_column() = 0;
};

struct LinkColumn;

struct LinkNode : LinkBase
{
    int row;
    LinkNode(int row, LinkBase *column) : row{row} {
        this->column = column;
        this->above = this;
        this->below = this;
        this->left = this;
        this->right = this;
    }
    virtual bool is_column() { return false; }
};

struct LinkColumn : LinkBase
{
    int size;
    int key;
    LinkColumn(int key) : size{0}, key{key} {
        this->above = this;
        this->below = this;
        this->left = this;
        this->right = this;
        this->column = this;
    }
    virtual bool is_column() { return true; }
};

inline void insert_below(LinkBase *node, LinkBase *other) {
    node->above = other;
    node->below = other->below;
    other->below = node;
    node->below->above = node;
}

inline void insert_after(LinkBase *node, LinkBase *other) {
    node->left = other;
    node->right = other->right;
    other->right = node;
    node->right->left = node;
}

inline void remove_from_row(LinkBase *node) {
    node->left->right = node->right;
    node->right->left = node->left;
}

inline void remove_from_column(LinkBase *node) {
    static_cast<LinkColumn*>(node->column)->size--;
    node->above->below = node->below;
    node->below->above = node->above;
}

inline void restore_in_row(LinkBase *node) {
    node->left->right = node;
    node->right->left = node;
}

inline void restore_in_column(LinkBase *node) {
    static_cast<LinkColumn*>(node->column)->size++;
    node->above->below = node;
    node->below->above = node;
}

void remove_row(LinkBase *node) {
    LinkBase *start = node->left;
    while (node != start) {
        remove_from_column(node);
        node = node->right;
    }
    remove_from_column(node);
}

void remove_column(LinkBase *node) {
    LinkBase *start = node->above;
    while (node != start) {
        remove_from_row(node);
        node = node->below;
    }
    remove_from_row(node);
}

void restore_row(LinkBase *node) {
    LinkBase *start = node->left;
    while (node != start) {
        restore_in_column(node);
        node = node->right;
    }
    restore_in_column(node);
}

void restore_column(LinkBase *node) {
    LinkBase *start = node->above;
    while (node != start) {
        restore_in_row(node);
        node = node->below;
    }
    restore_in_row(node);
}

void free_row(LinkBase *node) {
    node->left->right = nullptr;
    while (node != nullptr) {
        LinkBase *tmp = node;
        node = node->right;
        delete tmp;
    }
}

void free_column(LinkBase *node) {
    node->above->below = nullptr;
    while (node != nullptr) {
        LinkBase *tmp = node;
        node = node->below;
        delete tmp;
    }
}

inline int get_block_number(int cell_number) {
    int row = cell_number / SIZE;
    int col = cell_number % SIZE;
    int block_row = row / SIZE_ROOT;
    int block_col = col / SIZE_ROOT;
    return block_row * SIZE_ROOT + block_col;
}

void print_matrix(LinkBase *matrix) {
    LinkBase *col = matrix->right;
    while (col != matrix) {
        LinkBase *cell = col->below;
        cout << static_cast<LinkColumn*>(col)->key << " (" << static_cast<LinkColumn*>(col)->size << ") : ";
        while (cell != col) {
            cout << static_cast<LinkNode*>(cell)->row << " ";
            cell = cell->below;
        }
        cout << endl;
        col = col->right;
    }
}


LinkBase *make_matrix(int *table) {
    LinkBase *head = new LinkColumn{-1};

    // make first row with column blocks
    LinkBase *node = head;
    for (int i = 0; i < 4 * SIZE_SQUARE; i++) {
        node->right = new LinkColumn{i};
        node->right->left = node;
        node = node->right;
    }
    node->right = head;
    head->left = node;

    for (int cell_number = 0; cell_number < SIZE_SQUARE; cell_number++) {
        // for cell in matrix

        for (int value = 0; value < SIZE; value++) {
            // for value in cell
            int position;
            LinkBase *last_in_row;
            LinkBase *destination;

            // constraint - number in a cell
            position = cell_number;
            destination = head->right;
            while (position--) { destination = destination->right; }

            node = new LinkNode{cell_number * SIZE + value, destination->column};
            static_cast<LinkColumn*>(node->column)->size++;

            insert_below(node, destination->above);
            last_in_row = node;

            //constraint - value in a row
            position = SIZE_SQUARE + (cell_number / SIZE) * SIZE + value;
            destination = head->right;
            while (position--) { destination = destination->right; }

            node = new LinkNode{cell_number * SIZE + value, destination->column};
            static_cast<LinkColumn*>(node->column)->size++;

            insert_below(node, destination->above);
            insert_after(node, last_in_row);
            last_in_row = node;

            // constraint - value in a column
            position = SIZE_SQUARE * 2 + (cell_number % SIZE) * SIZE + value;
            destination = head->right;
            while (position--) { destination = destination->right; }

            node = new LinkNode{cell_number * SIZE + value, destination->column};
            static_cast<LinkColumn*>(node->column)->size++;

            insert_below(node, destination->above);
            insert_after(node, last_in_row);
            last_in_row = node;

            // constraint - value in a block
            position = SIZE_SQUARE * 3 + get_block_number(cell_number) * SIZE + value;
            destination = head->right;
            while (position--) { destination = destination->right; }

            node = new LinkNode{cell_number * SIZE + value, destination->column};
            static_cast<LinkColumn*>(node->column)->size++;

            insert_below(node, destination->above);
            insert_after(node, last_in_row);
        }
    }

    for (int cell_number = 0; cell_number < SIZE_SQUARE; cell_number++) {
        if (table[cell_number] != 0) {
            int value = table[cell_number];
            int row_in_matrix = cell_number * SIZE + value - 1;
            LinkBase *node = head;
            while (static_cast<LinkColumn*>(node)->key != cell_number) { node = node->right;}
            node = node->below;
            while (static_cast<LinkNode*>(node)->row != row_in_matrix) { node = node->below; }
            // cout << static_cast<LinkNode*>(node)->row << endl;
            // node's row relates to the value in the cell

            // delete rows that intersect current
            LinkBase *node_in_row = node;
            do {
                for (LinkBase *other_node = node_in_row->below; other_node != node_in_row;
                        other_node = other_node->below) {
                    if (!other_node->is_column()) {
                            // cout << "r r " << static_cast<LinkNode*>(other_node)->row << endl;
                        remove_row(other_node);
                        free_row(other_node);
                    }
                }
                node_in_row = node_in_row->right;
            } while (node_in_row != node);

            remove_row(node);

            // delete columns that marked in current row
            do {
                remove_column(node_in_row->column);
                free_column(node_in_row->column);
                node_in_row = node_in_row->right;
            } while (node_in_row != node);

            free_row(node);
        }
    }

    return head;
}

bool check(int *table) {
    set<int> found;
    // rows
    for (int i = 0; i < SIZE; i++) {
        found.clear();
        for (int j = 0; j < SIZE; j++) {
            int value = table[i * SIZE + j];
            if (value != 0) {
                if (found.find(value) == found.end()) {
                    found.insert(value);
                } else {
                    return false;
                }
            }
        }
    }
    // columns
    for (int j = 0; j < SIZE; j++) {
        found.clear();
        for (int i = 0; i < SIZE; i++) {
            int value = table[i * SIZE + j];
            if (value != 0) {
                if (found.find(value) == found.end()) {
                    found.insert(value);
                } else {
                    return false;
                }
            }
        }
    }
    // blocks
    for (int k = 0; k < SIZE_ROOT; k++)
    for (int m = 0; m < SIZE_ROOT; m++) {
        found.clear();
        for (int i = 0; i < SIZE_ROOT; i++)
        for (int j = 0; j < SIZE_ROOT; j++) {
            int value = table[(k * SIZE_ROOT + i) * SIZE + m * SIZE_ROOT + j];
            if (value != 0) {
                if (found.find(value) == found.end()) {
                    found.insert(value);
                } else {
                    return false;
                }
            }
        }
    }
    return true;
}

void algorithmX(LinkBase *matrix, int *table) {
    LinkBase *node = matrix->right;
    if (node == matrix) {
        // matrix has zero size
        cout << "Solution:" << endl;
        print_table(table);
        cout << "correct: " << check(table) << endl;
        auto end_t = chrono::steady_clock::now();
        auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_t - start_t);
        cout << "time: " << elapsed_ms.count() << endl;
        return;
    }

    // find column with the nodes quantity
    LinkBase *col = node;
    while (node != matrix) {
        int col_size = static_cast<LinkColumn*>(node)->size;
        if (col_size == 0) {
            return;
        }
        if (col_size < static_cast<LinkColumn*>(col)->size) {
            col = node;
        }
        node = node->right;
    }

    //cout << "column: " << static_cast<LinkColumn*>(col)->key << " " << static_cast<LinkColumn*>(col)->size << endl;

    // fork algorithm for all nodes in the column
    node = col->below;
    while (node != col) {
        stack <LinkBase*> deleted_rows;
        stack <LinkBase*> deleted_columns;

        // delete rows that intersect current
        LinkBase *other_node, *node_in_row = node;
        do {
            for (other_node = node_in_row->below; other_node != node_in_row;
                    other_node = other_node->below) {
                if (!other_node->is_column()) {
                    remove_row(other_node);
                    deleted_rows.push(other_node);
                }
            }
            node_in_row = node_in_row->right;
        } while (node_in_row != node);

        remove_row(node);
        deleted_rows.push(node);

        // delete columns that marked in current row
        do {
            remove_column(node_in_row->column);
            deleted_columns.push(node_in_row->column);
            node_in_row = node_in_row->right;
        } while (node_in_row != node);


        // recursion goes brrr
        int assumption = static_cast<LinkNode*>(node)->row;
        answer.push(assumption);
        table[assumption / SIZE] = assumption % SIZE + 1;
        algorithmX(matrix, table);
        answer.pop();

        // restore matrix
        while (!deleted_columns.empty()) {
            other_node = deleted_columns.top();
            deleted_columns.pop();
            restore_column(other_node);
        }

        while (!deleted_rows.empty()) {
            other_node = deleted_rows.top();
            deleted_rows.pop();
            restore_row(other_node);
        }

        node = node->below;
    }
}

void free_memory(int *table, LinkBase *matrix) {
    delete[] table;
    LinkBase *col = matrix->right;
    while (col != matrix) {
        LinkBase *node = col->below;
        while (node != col) {
            LinkBase *tmp = node;
            node = node->below;
            delete tmp;
        }
        LinkBase *tmp = col;
        col = col->right;
        delete tmp;
    }
    delete matrix;
}

int main() {
    start_t = chrono::steady_clock::now();

    int *table;

    read_input("in16b16.txt", table);
    print_table(table);
    cout << endl;

    LinkBase *matrix =  make_matrix(table);

    //print_matrix(matrix);

    algorithmX(matrix, table);

    free_memory(table, matrix);

    return 0;
}
