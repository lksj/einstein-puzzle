#include "compressor.h"
#include "exceptions.h"
#include "unicode.h"
#include "table.h"


static std::string sourceFile, outputFile;
static bool verbose = false;
static bool showDeps = false;


static void printHelp(int terminate)
{
    std::cerr << "USAGE:" << std::endl;
    std::cerr << "  mkres [options]" << std::endl;
    std::cerr << "OPTIONS:" << std::endl;
    std::cerr << "  --verbose        print more messages" << std::endl;
    std::cerr << "  --deps           print make(1) dependencies" << std::endl;
    std::cerr << "  --source <file>  read source file" << std::endl;
    std::cerr << "  --output <file>  write result to file" << std::endl;
    std::cerr << "  --version        show version number" << std::endl;
    std::cerr << "  --help           this help screen" << std::endl;
    if (terminate >= 0)
        exit(terminate);
}


static void printVersion(int terminate)
{
    std::cout << "Resource File Compressor" << std::endl;
    std::cout << "Version 1.2" << std::endl;
    std::cout << "Copyright (c) 2003-2005 Alexander Babichev" << std::endl;
    if (terminate >= 0)
        exit(terminate);
}


static void parseArgs(int argc, char *argv[])
{
    int i;
    
    if (argc == 1)
        printHelp(0);

    for (i = 1; i < argc; i++) {
        if (! argv[i])
            continue;
        if ((! strcmp(argv[i], "--source")) && (i < argc - 1))
            sourceFile = std::string(argv[++i]);
        else if ((! strcmp(argv[i], "--output")) && (i < argc - 1))
            outputFile = std::string(argv[++i]);
        else if (! strcmp(argv[i], "--help"))
            printHelp(0);
        else if (! strcmp(argv[i], "--version"))
            printVersion(0);
        else if (! strcmp(argv[i], "--verbose"))
            verbose = true;
        else if (! strcmp(argv[i], "--deps"))
            showDeps = true;
        else {
            std::cerr << "Invalid option '" << argv[i] << "'" << std::endl;
            printHelp(1);
        }
    }

    if (! sourceFile.length()) {
        std::cerr << "Source file not specified" << std::endl;
        exit(1);
    }

    if (! outputFile.length()) {
        std::cerr << "Output file not specified" << std::endl;
        exit(1);
    }
}


static ResourceCompressor compressor;


static void parseFile(const std::string &fileName)
{
    Table table(fileName);
    
    compressor.setPriority(table.getInt(L"priority", 1000));

    Table *res = table.getTable(L"resources");
    if (res) {
        for (Table::Iterator i = res->begin(); i != res->end(); i++) {
            Value *value = (*i).second;
            if ((! value) || (! (Value::Table == value->getType())))
                throw Exception(L"Invalid resource entry");
            Table *t = value->asTable();
            const std::wstring &name = t->getString(L"name");
            if (0 >= name.length())
                throw Exception(L"Unnamed resource entry");
            const std::wstring &format = t->getString(L"format");
            Formatter *formatter = formatRegistry.get(format);
            if ((0 < format.length()) && (! formatter))
                throw Exception(L"Unknown format '" + format + L"'");
            compressor.add(Entry(name, t->getInt(L"compr", 9), 
                        t->getString(L"group", L""), 
                        t->getString(L"file", name), formatter));
        }
    }
}



int main(int argc, char *argv[])
{
    int res = 0;
    //lua_State *lua = NULL;
    parseArgs(argc, argv);
    
    try {
        parseFile(sourceFile);
        if (! showDeps)
            compressor.compress(outputFile, verbose);
        else
            compressor.printDeps(outputFile, sourceFile);
    } catch (Exception &e) {
        std::cerr << e.getMessage() << std::endl;
        res = 1;
    } catch (std::string &s) {
        std::cerr << s << std::endl;
        res = 1;
    } catch (...) {
        std::cerr << L"Error: unknown exception" << std::endl;
        res = 1;
    }

/*    if (lua) {
        lua_setgcthreshold(lua, 0);
        lua_close(lua);
    }*/
    
    return res;
}

