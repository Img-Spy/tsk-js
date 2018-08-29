
#include "tsk_js_i.h"
#include "stdio.h"

namespace tsk_js {

using namespace v8;

class TskJsPrint {
    class TskJsPrintObject {
    public:
        TskJsPrintObject(TskJsPrint *printer): _printer(printer)
        { this->_printer->PrintText("{"); }
        ~TskJsPrintObject()
        { this->_printer->PrintText("}"); }

    private:
        TskJsPrint *_printer;
    };

    class TskJsPrintArray {
    public:
        TskJsPrintArray(TskJsPrint *printer): _printer(printer)
        { this->_printer->PrintText("["); }
        ~TskJsPrintArray()
        { this->_printer->PrintText("]"); }

    private:
        TskJsPrint *_printer;
    };

public:
    TskJsPrint(Isolate *isolate, FILE *file = stdout) :
        _isolate(isolate), _file(file) {}

    void Print(Local<Value> val); 

private:
    Isolate *GetIsolate() { return this->_isolate; }

    void PrintText(const char *txt) { fprintf(this->_file, "%s", txt); }
    void PrintObject(Local<Object> obj);
    void PrintArray(Local<Array> val);
    void PrintFunction(Local<Function> val);
    void PrintDefault(Local<Value> val);

    Isolate *_isolate;
    FILE *_file;
};

/******************************************************************************/

void
TskJsPrint::PrintObject(Local<Object> obj)
{
    EscapableHandleScope handle_scope(this->GetIsolate());
    TskJsPrint::TskJsPrintObject level(this);

    Local<Array> keys = obj->GetPropertyNames();
    uint32_t length = keys->Length();

    for (uint32_t i = 0; i < length; i++) {
        Local<Value> key = keys->Get(i);
        Local<Value> val = obj->Get(key);

        this->PrintDefault(key);
        fprintf(this->_file, " = ");
        this->Print(val);

        if (i < length - 1)    fprintf(this->_file, ", ");
    }
}

void
TskJsPrint::PrintArray(Local<Array> arr)
{
    EscapableHandleScope handle_scope(this->GetIsolate());
    TskJsPrint::TskJsPrintArray level(this);

    uint32_t length = arr->Length();

    for (uint32_t i = 0; i < length; i++) {
        Local<Value> val = arr->Get(i);

        this->Print(val);
        if (i < length - 1)    fprintf(this->_file, ", ");
    }
}

void
TskJsPrint::PrintFunction(Local<Function> func)
{
    fprintf(this->_file, "[Function]");
}

void
TskJsPrint::PrintDefault(Local<Value> val)
{
    EscapableHandleScope handle_scope(this->GetIsolate());
    String::Utf8Value string(val->ToString());

    if (val->IsString())
        fprintf(this->_file, "\"%s\"", (*string));
    else
        fprintf(this->_file, "%s", (*string));
}

void
TskJsPrint::Print(Local<Value> val)
{
    EscapableHandleScope handle_scope(this->GetIsolate());

    if (val.IsEmpty())
        return;
    if (val->IsArray())
        return this->PrintArray(Local<Array>::Cast(val));
    if (val->IsFunction())
        return this->PrintFunction(Local<Function>::Cast(val));
    if (val->IsDate())
        return this->PrintDefault(val);
    if (val->IsObject())
        return this->PrintObject(val->ToObject());

    this->PrintDefault(val);
}

void
_tsk_js_Print(Local<Value> val)
{
    Isolate *isolate = Isolate::GetCurrent();

    TskJsPrint printer(isolate);
    printer.Print(val);
    printf("\n");
}


}
