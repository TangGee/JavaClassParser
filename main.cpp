#include <iostream>
#include "type.h"
#include <fstream>
#include <vector>


struct cp_info{
    u1 tag;
};

struct utf8_cpinfo : public cp_info {
    std::string value;
};

typedef std::shared_ptr<cp_info>  cp_info_ptr;


enum Constant_Pool_Type{
    Utf8 = 1,
    Integet = 3,
    Float = 4,
    Long = 5,
    Double = 6,
    Class = 7,
    String = 8,
    FieldRef = 9,
    MethodRef = 10,
    InterfaceRef = 11,
    NameAndType = 12,
    MethodHandle = 15,
    MethodType = 16,
    InvokeDynamic = 18,
};

struct attribute {

};

struct attribute_info{
    u2 name_index;
    u4 length;
};

typedef std::shared_ptr<attribute_info>  attribute_ptr;


struct exception {
     u2 start_pc;
     u2 end_pc;
     u2 handler_pc;
     u2 catch_type;
};

struct code_attribute_info : public attribute_info {
 u2 max_stack;
 u2 max_locals;
 u4 code_length;
 char* code;
 u2 exception_table_length;
 std::vector<exception> exception_table;
 std::vector<attribute_ptr> attributes;

 ~code_attribute_info() {
     delete[] code;
 }
};

struct line_entry {
    u2 start_pc;
    u2 line_number;
};

struct line_attribute_info: public attribute_info {
    u2 line_number_table_length;
    std::vector<line_entry> line_entries;
};

struct field_info {
    u2 access_flags;
    u2 name_index;
    u2 descriptor;
    u2 attribute_count;
    std::vector<attribute_ptr> attributes;
};

struct method_info {
    u2 access_flags;
    u2 name_index;
    u2 descriptor;
    u2 attribute_count;
    std::vector<attribute_ptr> attributes;
};

struct ClassFile {
    u4 magic;
    u2 minjor_version;
    u2 major_version;
    u2  constant_pool_count;
    std::vector<cp_info_ptr> constant_pool;
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    u2 interface_count;
    std::vector<u2> interfaces;
    u2 field_count;
    std::vector<field_info> field_infos;
    u2 method_count;
    std::vector<method_info> method_infos;
    u2 attribute_count;
    std::vector<attribute_ptr> attribute_infos;

};

void readByte(std::istream &stream, void *buf, int size) {
    char b;
    char *readBuf = reinterpret_cast<char *>(buf);
    for (int i = 0; i < size; ++i) {
        stream.read(&b, 1);
        readBuf[size - i - 1] = b;
    }
}

void readString(std::istream &stream, void *buf, int size){
    char *readBuf = reinterpret_cast<char *>(buf);
    stream.read(readBuf, size);
}

void readConstanPool(std::istream &stream, u2 count, std::vector<cp_info_ptr> &cp_infos) {
    for (int i = 1; i < count; ++i) {
        u1 tag;
        bool hit = true;

        readByte(stream, &tag, sizeof(tag));

        std::cout << "constant pool type " << (int)tag << std::endl;

        cp_info *cpInfo;

        switch (tag) {
            case MethodRef: {
                u2 v;
                readByte(stream, &v, sizeof(u2));
                readByte(stream, &v, sizeof(u2));

                cpInfo = new cp_info;

                break;
            }
            case String: {
                u2 v;
                readByte(stream, &v, sizeof(u2));
                cpInfo = new cp_info;
                break;
            }
            case FieldRef: {
                u2 v;
                readByte(stream, &v, sizeof(u2));
                readByte(stream, &v, sizeof(u2));
                cpInfo = new cp_info;
                break;
            }
            case Class: {
                u2 v;
                readByte(stream, &v, sizeof(u2));
                cpInfo = new cp_info;
                break;
            }
            case Utf8:{
                u2 v;
                readByte(stream, &v, sizeof(u2));
                auto len = (int) (v);
                std::cout << "utf-8 len " << len << std::endl;
                char str[len + 1];
                str[len] = '\0';
                readString(stream, str, len);
                std::cout << str << std::endl;

                auto *ucpInfo = new utf8_cpinfo;
                ucpInfo->value = str;
                cpInfo = ucpInfo;
                break;
            }
            case NameAndType: {
                u2 v;
                readByte(stream, &v, sizeof(u2));
                readByte(stream, &v, sizeof(u2));
                cpInfo = new cp_info;
                break;
            }

            default:
                hit = false;
                break;
        }

        if (hit) {
            if (!cpInfo) {
                cpInfo = new cp_info;
            }
            cpInfo->tag = tag;
            cp_infos.emplace_back(cpInfo);
        }
    }
}

void readInterface(std::istream &stream, std::vector<u2> &interfaces, u2 count) {
    for (int i = 0; i < count; ++i) {
        u2 interface;
        readByte(stream, &interface, sizeof(interface));
        interfaces.emplace_back(interface);

    }
}

void readExceptions(std::istream &stream, std::vector<exception> &exceptions, u2 count) {
    for (int i = 0; i < count ; ++i) {
        exception e;
        readByte(stream, &e.start_pc, sizeof(e.start_pc));
        readByte(stream, &e.end_pc, sizeof(e.end_pc));
        readByte(stream, &e.handler_pc, sizeof(e.handler_pc));
        readByte(stream, &e.catch_type, sizeof(e.catch_type));
        exceptions.emplace_back(e);
    }
}

void readAttributes(ClassFile &classFile, std::istream &stream, std::vector<attribute_ptr> &attributes, u2 attribute_count);

void readCodeAttribute(ClassFile &classFile, std::istream &stream, code_attribute_info &attr) {
    readByte(stream, &attr.max_stack, sizeof(attr.max_stack));
    std::cout << "code max stack " << attr.max_stack << std::endl;


    readByte(stream, &attr.max_locals, sizeof(attr.max_locals));
    std::cout << "code max locals " << attr.max_locals << std::endl;

    readByte(stream, &attr.code_length, sizeof(attr.code_length));
    std::cout << "code length " << attr.code_length << std::endl;

    attr.code = new char[attr.code_length];
    readByte(stream, attr.code, attr.code_length);

    readByte(stream, &attr.exception_table_length, sizeof(attr.exception_table_length));
    std::cout << "code exception length " << attr.exception_table_length << std::endl;

    readExceptions(stream, attr.exception_table, attr.exception_table_length);

    u2 ac;
    readByte(stream, &ac, sizeof(ac));
    std::cout << "code attribute count " << ac << std::endl;

    readAttributes(classFile, stream, attr.attributes, ac);

}

void readLineTableAttribute(std::istream &stream, line_attribute_info &attribute_info) {
    readByte(stream, &attribute_info.line_number_table_length, sizeof(attribute_info.line_number_table_length));
    std::cout << "line table length " << attribute_info.line_number_table_length << std::endl;

    for (int i = 0; i < attribute_info.line_number_table_length; ++i) {
        line_entry entry;
        readByte(stream, &entry.start_pc, sizeof(entry.start_pc));
        std::cout << "line entry start pc " << entry.start_pc << std::endl;

        readByte(stream, &entry.line_number, sizeof(entry.line_number));
        std::cout << "line entry line number " << entry.line_number << std::endl;
        attribute_info.line_entries.emplace_back(entry);
    }

}

void readAttributes(ClassFile &classFile, std::istream &stream, std::vector<attribute_ptr> &attributes, u2 attribute_count) {
    for (int i = 0; i < attribute_count; ++i) {
        u2 index;
        readByte(stream, &index, sizeof(index));

        std::cout << "attribute type index " << (int)index << std::endl;
        cp_info_ptr cp = classFile.constant_pool[index];

        if (cp->tag != Utf8) {
            abort();
        }
        auto *ucpinfo = reinterpret_cast<utf8_cpinfo *>(cp.get());
        std::cout << "attribute type name " << ucpinfo->value << std::endl;


        u4 byteCount;
        readByte(stream, &byteCount, sizeof(byteCount));

        if (ucpinfo->value == "Code") {
            auto codeAttributeInfo = new code_attribute_info;
            codeAttributeInfo->name_index = index;
            codeAttributeInfo->length = byteCount;
            readCodeAttribute(classFile, stream, *codeAttributeInfo);
            attributes.emplace_back(codeAttributeInfo);
        } if (ucpinfo->value == "LineNumberTable") {
            auto lineTableInfo = new line_attribute_info;
            lineTableInfo->name_index = index;
            lineTableInfo->length = byteCount;
            readLineTableAttribute(stream, *lineTableInfo);
            attributes.emplace_back(lineTableInfo);
        }
    }
}

void readFieldInfos(ClassFile &classFile, std::istream &stream, std::vector<field_info> &fieldInfos, u2 count) {
    for (int i = 0; i < count; ++i) {
        field_info fieldInfo;

        readByte(stream, &fieldInfo.access_flags, sizeof(fieldInfo.access_flags));
        std::cout << "field access flags " << fieldInfo.access_flags << std::endl;

        readByte(stream, &fieldInfo.name_index, sizeof(fieldInfo.name_index));
        std::cout << "field name index " << fieldInfo.name_index << std::endl;

        readByte(stream, &fieldInfo.descriptor, sizeof(fieldInfo.descriptor));
        std::cout << "field descriptor " << fieldInfo.descriptor << std::endl;

        readByte(stream, &fieldInfo.attribute_count, sizeof(fieldInfo.attribute_count));
        std::cout << "field attribute count " << fieldInfo.attribute_count << std::endl;

        readAttributes(classFile, stream, fieldInfo.attributes, fieldInfo.attribute_count);

        fieldInfos.emplace_back(fieldInfo);
    }
}

void readMethodInfos(ClassFile &classFile, std::istream &stream, std::vector<method_info> &methodInfos, u2 count) {
    for (int i = 0; i < count; ++i) {
        method_info methodInfo;
        readByte(stream, &methodInfo.access_flags, sizeof(methodInfo.access_flags));
        std::cout << "method access flags " << methodInfo.access_flags << std::endl;

        readByte(stream, &methodInfo.name_index, sizeof(methodInfo.name_index));
        std::cout << "method name index " << methodInfo.name_index << std::endl;

        readByte(stream, &methodInfo.descriptor, sizeof(methodInfo.descriptor));
        std::cout << "method descriptor " << methodInfo.descriptor << std::endl;

        readByte(stream, &methodInfo.attribute_count, sizeof(methodInfo.attribute_count));
        std::cout << "method attribute count " << methodInfo.attribute_count << std::endl;

        readAttributes(classFile, stream, methodInfo.attributes, methodInfo.attribute_count);
        methodInfos.emplace_back(methodInfo);
    }
}


int main() {
    std::ifstream ifs;
    ifs.open("/Users/ermei/art/class/TestClass.class", std::ios::in);
    if (!ifs.is_open()) {
        std::cout << "open file error " << std::endl;
    }

    auto *classFile = new ClassFile;

    if (ifs.is_open() && !ifs.eof()) {
        ifs.read(reinterpret_cast<char *>(&classFile->magic), sizeof(classFile->magic));
        if (classFile->magic != 3199925962) {
            std::cout << "magic error" << std::endl;
            return -1;
        }

        readByte(ifs, &classFile->minjor_version, sizeof(classFile->minjor_version));
        std::cout  << "minjor version: " << classFile->minjor_version << std::endl;


        readByte(ifs, &classFile->major_version, sizeof(classFile->major_version));
        std::cout << "major version: " << classFile->major_version << std::endl;

        readByte(ifs, &classFile->constant_pool_count, sizeof(classFile->constant_pool_count));
        std::cout << "constant pool count: " << classFile->constant_pool_count << std::endl;

        classFile->constant_pool.emplace_back(new cp_info);
        readConstanPool(ifs, classFile->constant_pool_count, classFile->constant_pool);

        readByte(ifs, &classFile->access_flags, sizeof(classFile->access_flags));
        std::cout << "access flags: " << classFile->access_flags << std::endl;

        readByte(ifs, &classFile->this_class, sizeof(classFile->this_class));
        std::cout << "this class: " << classFile->this_class << std::endl;

        readByte(ifs, &classFile->super_class, sizeof(classFile->super_class));
        std::cout << "super class: " << classFile->super_class << std::endl;

        readByte(ifs, &classFile->interface_count, sizeof(classFile->interface_count));
        std::cout << "interface count: " << classFile->interface_count << std::endl;

        if (classFile->interface_count > 0) {
            readInterface(ifs, classFile->interfaces, classFile->interface_count);
        }

        readByte(ifs, &classFile->field_count, sizeof(classFile->field_count));
        std::cout << "field count: " << classFile->field_count << std::endl;

        if (classFile->field_count > 0) {
            readFieldInfos(*classFile, ifs, classFile->field_infos, classFile->field_count);
        }

        readByte(ifs, &classFile->method_count, sizeof(classFile->method_count));
        std::cout << "method count: " << classFile->method_count << std::endl;

        if (classFile->method_count > 0) {
            readMethodInfos(*classFile, ifs, classFile->method_infos, classFile->method_count);
        }

        readByte(ifs, &classFile->attribute_count, sizeof(classFile->attribute_count));
        std::cout << "attribute count: " << classFile->attribute_count << std::endl;

        readAttributes(*classFile, ifs, classFile->attribute_infos, classFile->attribute_count);

        delete classFile;
    }
}