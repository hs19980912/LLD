# Text Editor Undo/Redo


> This is a classic application of the `Command pattern` and `Stack` data structures.

```cpp
class TextEditor {
private:
    string content;
    stack<shared_ptr<Command>> undoStack;
    stack<shared_ptr<Command>> redoStack;

public:
    TextEditor() : content("") {}

    // Get current text content
    string getContent() const {
        return content;
    }

    // Execute a command and add it to undo stack
    void executeCommand(shared_ptr<Command> command) {
        command->execute();
        undoStack.push(command);
        
        // Clear redo stack when a new command is executed
        while (!redoStack.empty()) {
            redoStack.pop();
        }
    }

    // Undo the last command
    bool undo() {
        if (undoStack.empty()) {
            return false;
        }

        auto command = undoStack.top();
        undoStack.pop();
        command->undo();
        redoStack.push(command);
        return true;
    }

    // Redo the last undone command
    bool redo() {
        if (redoStack.empty()) {
            return false;
        }

        auto command = redoStack.top();
        redoStack.pop();
        command->execute();
        undoStack.push(command);
        return true;
    }

    // Command interface
    class Command {
    public:
        virtual ~Command() = default;
        virtual void execute() = 0;
        virtual void undo() = 0;
    };

    // Insert text at a position
    class InsertCommand : public Command {
    private:
        TextEditor& editor;
        size_t position;
        string textToInsert;

    public:
        InsertCommand(TextEditor& ed, size_t pos, const string& text)
            : editor(ed), position(pos), textToInsert(text) {}

        void execute() override {
            editor.content.insert(position, textToInsert);
        }

        void undo() override {
            editor.content.erase(position, textToInsert.length());
        }
    };

    // Delete text from a position
    class DeleteCommand : public Command {
    private:
        TextEditor& editor;
        size_t position;
        size_t length;
        string deletedText;

    public:
        DeleteCommand(TextEditor& ed, size_t pos, size_t len)
            : editor(ed), position(pos), length(len) {
            // Store the text to be deleted for undo operation
            if (position < editor.content.length()) {
                deletedText = editor.content.substr(position, length);
            }
        }

        void execute() override {
            if (position < editor.content.length()) {
                deletedText = editor.content.substr(position, length);
                editor.content.erase(position, length);
            }
        }

        void undo() override {
            editor.content.insert(position, deletedText);
        }
    };

    // Replace text at a position
    class ReplaceCommand : public Command {
    private:
        TextEditor& editor;
        size_t position;
        size_t length;
        string newText;
        string oldText;

    public:
        ReplaceCommand(TextEditor& ed, size_t pos, size_t len, const string& text)
            : editor(ed), position(pos), length(len), newText(text) {
            if (position < editor.content.length()) {
                oldText = editor.content.substr(position, length);
            }
        }

        void execute() override {
            if (position < editor.content.length()) {
                oldText = editor.content.substr(position, min(length, editor.content.length() - position));
                editor.content.replace(position, length, newText);
            }
        }

        void undo() override {
            editor.content.replace(position, newText.length(), oldText);
        }
    };

    // Convenience methods to create and execute commands
    void insert(size_t position, const string& text) {
        executeCommand(make_shared<InsertCommand>(*this, position, text));
    }

    void deleteText(size_t position, size_t length) {
        executeCommand(make_shared<DeleteCommand>(*this, position, length));
    }

    void replace(size_t position, size_t length, const string& newText) {
        executeCommand(make_shared<ReplaceCommand>(*this, position, length, newText));
    }
};

// Demo usage
int main() {
    TextEditor editor;
    
    // Initial text
    editor.insert(0, "Hello world!");
    cout << "Initial: " << editor.getContent() << endl;
    
    // Insert operation
    editor.insert(5, " beautiful");
    cout << "After insert: " << editor.getContent() << endl;
    
    // Delete operation
    editor.deleteText(13, 5);
    cout << "After delete: " << editor.getContent() << endl;
    
    // Replace operation
    editor.replace(0, 5, "Greetings");
    cout << "After replace: " << editor.getContent() << endl;
    
    // Undo operations
    editor.undo(); // Undo replace
    cout << "After undo replace: " << editor.getContent() << endl;
    
    editor.undo(); // Undo delete
    cout << "After undo delete: " << editor.getContent() << endl;
    
    editor.undo(); // Undo insert
    cout << "After undo insert: " << editor.getContent() << endl;
    
    // Redo operations
    editor.redo(); // Redo insert
    cout << "After redo insert: " << editor.getContent() << endl;
    
    editor.redo(); // Redo delete
    cout << "After redo delete: " << editor.getContent() << endl;
    
    // Add new command after undo/redo
    editor.insert(editor.getContent().length(), " Updated!");
    cout << "New command after undo/redo: " << editor.getContent() << endl;
    
    // Try to redo (should fail as redo stack is cleared)
    bool redoSuccess = editor.redo();
    cout << "Redo after new command: " << (redoSuccess ? "succeeded" : "failed") << endl;
    
    return 0;
}

```

## Design Explanation

This implementation uses several key design concepts:

1. **Command Pattern**: Each text operation (insert, delete, replace) is encapsulated in a command object that knows how to execute and undo itself.

2. **Undo and Redo Stacks**: Two stacks track the history of commands for undo and redo operations.

3. **RAII Principles**: Using `shared_ptr` ensures proper memory management of command objects.

4. **Command Interface**: An abstract base class defines the interface for all commands.

### Key Features

- **Fully Featured Text Operations**: Insert, delete, and replace functionality
- **Unlimited History**: No artificial limit on undo/redo operations
- **Memory Efficient**: Only stores the differences between states, not entire document copies
- **Type-Safe**: Modern C++ practices with proper encapsulation
- **Extensible**: Easy to add new command types

### Implementation Details

1. When a text operation occurs, a corresponding command object is created, executed, and pushed onto the undo stack.

2. When an undo operation is requested, the top command from the undo stack is popped, its undo method is called, and the command is pushed onto the redo stack.

3. When a redo operation is requested, the top command from the redo stack is popped, its execute method is called, and the command is pushed back onto the undo stack.

4. When a new command is executed after any undo operations, the redo stack is cleared, which is standard behavior in most text editors.

This design can be easily extended to handle more complex text operations like formatting, cursor position tracking, or multi-selection edits by adding more specialized command classes.