// Forward declaration
class TextEditor;

// Command base class
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// Insert text command
class InsertCommand : public Command {
private:
    TextEditor* editor;
    size_t position;
    string text;
    
public:
    InsertCommand(TextEditor* ed, size_t pos, const string& txt);
    void execute() override;
    void undo() override;
};

// Delete text command
class DeleteCommand : public Command {
private:
    TextEditor* editor;
    size_t position;
    string deletedText;
    
public:
    DeleteCommand(TextEditor* ed, size_t pos, size_t len);
    void execute() override;
    void undo() override;
};

// Text editor class
class TextEditor {
private:
    string content;
    stack<shared_ptr<Command>> undoStack;
    stack<shared_ptr<Command>> redoStack;
    
public:
    // Friend classes to access content
    friend class InsertCommand;
    friend class DeleteCommand;
    
    // Constructor
    TextEditor() : content("") {}
    
    // Get current content
    string getContent() const { return content; }
    
    // Execute a command
    void executeCommand(shared_ptr<Command> cmd) {
        cmd->execute();
        undoStack.push(cmd);
        redoStack.clear();
    }
    
    // Undo last command
    bool undo() {
        if (undoStack.empty()) return false;
        
        auto cmd = undoStack.top();
        undoStack.pop();
        cmd->undo();
        redoStack.push(cmd);
        return true;
    }
    
    // Redo last undone command
    bool redo() {
        if (redoStack.empty()) return false;
        
        auto cmd = redoStack.top();
        redoStack.pop();
        cmd->execute();
        undoStack.push(cmd);
        return true;
    }
    
    // Convenience methods
    void insert(size_t pos, const string& text) {
        executeCommand(make_shared<InsertCommand>(this, pos, text));
    }
    
    void deleteText(size_t pos, size_t len) {
        executeCommand(make_shared<DeleteCommand>(this, pos, len));
    }
};

// Command implementations
InsertCommand::InsertCommand(TextEditor* ed, size_t pos, const string& txt) 
    : editor(ed), position(pos), text(txt) {}

void InsertCommand::execute() {
    editor->content.insert(position, text);
}

void InsertCommand::undo() {
    editor->content.erase(position, text.length());
}

DeleteCommand::DeleteCommand(TextEditor* ed, size_t pos, size_t len) 
    : editor(ed), position(pos) {
    if (position < editor->content.length()) {
        size_t actualLen = min(len, editor->content.length() - position);
        deletedText = editor->content.substr(position, actualLen);
    }
}

void DeleteCommand::execute() {
    if (!deletedText.empty()) {
        editor->content.erase(position, deletedText.length());
    }
}

void DeleteCommand::undo() {
    editor->content.insert(position, deletedText);
}

int main() {
    TextEditor editor;
    
    editor.insert(0, "Hello world");
    cout << "After insert: " << editor.getContent() << endl;
    
    editor.insert(5, " beautiful");
    cout << "After second insert: " << editor.getContent() << endl;
    
    editor.deleteText(0, 6);
    cout << "After delete: " << editor.getContent() << endl;
    
    editor.undo();
    cout << "After undo: " << editor.getContent() << endl;
    
    editor.redo();
    cout << "After redo: " << editor.getContent() << endl;
    
    return 0;
}