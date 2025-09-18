/*
 * Auto-added header
 * File: exceptions.h
 * Author: camradeling
 * Email: camradeling@gmail.com
 * 2025
 */
//------------------------------------------------------------------------------------------------------------------------------
#ifndef ELFMAN_EXCEPTIONS_H
#define ELFMAN_EXCEPTIONS_H
//------------------------------------------------------------------------------------------------------------------------------
#include <string>
#include <stdexcept>
//------------------------------------------------------------------------------------------------------------------------------
class exception_base : public std::runtime_error {
public:
    exception_base()
    : std::runtime_error(std::string()) { }

    exception_base(const std::string& message)
    : std::runtime_error(message) { }

    exception_base(const char* message)
    : std::runtime_error(message) { }
};
//------------------------------------------------------------------------------------------------------------------------------
class malformed_object : public exception_base {
public:
    malformed_object() : exception_base("Malformed object") { }
    malformed_object(const std::string& message) : exception_base(message) { }
};
//------------------------------------------------------------------------------------------------------------------------------
class serialization_error : public exception_base {
public:
    serialization_error() : exception_base("Serialization error") { }
};
//------------------------------------------------------------------------------------------------------------------------------
#endif/*ELFMAN_EXCEPTIONS_H*/