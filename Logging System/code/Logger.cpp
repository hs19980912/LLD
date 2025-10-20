
class ILogger{
public:
    virtual void log(const string& msg) = 0;
    virtual ~ILogger() = default;
};

class ConsoleLogger : public ILogger {
    void log(const string& msg) override {
        // console log impl
    }
};

class DiskLogger : public ILogger {
    void log(const string& msg) override {
        // Disk logging impl
    }
};