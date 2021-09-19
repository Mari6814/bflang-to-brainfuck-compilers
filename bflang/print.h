//
// Created by Marian Plivelic on 21.05.17.
//

#ifndef BFLANG_PRINT_H_H
#define BFLANG_PRINT_H_H
#include <iostream>
#include <vector>


inline std::ostream &ossprint(std::ostream &os, char sep) { return os; }

template<typename T, typename... Args>
inline std::ostream &ossprint(std::ostream &os, char sep, const T &msg, const Args &... args) {
    os << msg;
    if (sizeof...(Args))
        os << sep;
    return ossprint(os, sep, args...);
}

inline std::ostream &ossprintln(std::ostream &os, char sep) { return os << std::endl; }

template<typename T, typename... Args>
inline std::ostream &ossprintln(std::ostream &os, char sep, const T &msg, const Args &... args) {
    os << msg;
    if (sizeof...(Args))
        os << sep;
    return ossprintln(os, sep, args...);
}


inline std::ostream &osprintln(std::ostream &os) { return ossprintln(os, ' '); }

template<typename T, typename... Args>
inline std::ostream &osprintln(std::ostream &os, const T &msg, const Args &... args) {
    return ossprintln(os, ' ', msg, args...);
}

inline std::ostream &osprint(std::ostream &os) { return ossprint(os, ' '); }

template<typename T, typename... Args>
inline std::ostream &osprint(std::ostream &os, const T &msg, const Args &... args) {
    return ossprint(os, ' ', msg, args...);
}


inline std::ostream &println() { return osprintln(std::cout); }

template<typename T, typename... Args>
inline std::ostream &println(const T &msg, const Args &... args) {
    return osprintln(std::cout, msg, args...);
}

inline std::ostream &print() { return osprint(std::cout); }

template<typename T, typename... Args>
inline std::ostream &print(const T &msg, const Args &... args) {
    return osprint(std::cout, msg, args...);
}


inline std::ostream &errprintln() { return osprintln(std::cerr); }

template<typename T, typename... Args>
inline std::ostream &errprintln(const T &msg, const Args &... args) {
    return osprintln(std::cerr, msg, args...);
}

inline std::ostream &errprint() { return osprint(std::cerr); }

template<typename T, typename... Args>
inline std::ostream &errprint(const T &msg, const Args &... args) {
    return osprint(std::cerr, msg, args...);
}


#endif //BFLANG_PRINT_H_H
