#include <string>
#include <memory>
using namespace std;

// Step 1: Make Logger Interface and Concrete logger classes
/*----------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------*/

// Logger Factory

enum class LOGGER_TYPE{
    CONSOLE,
    DISK,
    DB
};

class LoggerFactory {
public:
    static std::unique_ptr<ILogger> getLogger(LOGGER_TYPE type){
        switch (type)
        {
            case LOGGER_TYPE::CONSOLE :
                return make_unique<ConsoleLogger>();
                break;
            
            default:
                break;
        }
    }
};

/*----------------------------------------------------------------------*/
// Making the Factory Singleton

class Factory{
private:

public:
    static getLogger(LOGGER_TYPE type)
};


int main(){
    auto logger = Logger::getLogger(LOGGER_TYPE::CONSOLE);

    string msg = "Console log message";
    logger->log(msg);

}