//
// Created by Marian Plivelic on 09.04.18.
//

#ifndef BRAINFUCKLANG_SYMBOL_H
#define BRAINFUCKLANG_SYMBOL_H

#include "brainfuck.h"
#include "Location.h"
#include "forward.h"

namespace brainfuck::compiler {
    struct Symbol {
        /** @brief Parent symbol table */
        Environment *parent = nullptr;

        /** @brief Name of the symbol */
        id_t id = "<unnamed>";

        /** @brief Declaration locations */
        std::vector<Location> declarationLocations;

        /** @brief Locations of read-access (should not be 0 at the end) */
        std::vector<Location> accessedLocations;

        /** @brief Location of symbol completion/implementation */
        Location implementationLocation;

        /** @brief Wether the symbol has a name or not */
        bool isAnonymous = false;

        /** @brief Symbol completed flag */
        bool isComplete = false;

        /** @return Simple Qualified Id list until root */
        virtual std::string qualifiedId() const;

        /** @brief Marks symbol as completed and writes the implementation location */
        void setCompleted(antlr4::ParserRuleContext *implementationLocationContext);

        /** @return Location of first declaration */
        Location firstDeclaration() const {
            assert(!declarationLocations.empty());
            return declarationLocations.front();
        }

        template<typename SymbolType>
        SymbolType *as() {
            return dynamic_cast<SymbolType*>(this);
        }

        template<typename SymbolType>
        const SymbolType *as() const {
            return dynamic_cast<const SymbolType*>(this);
        }

        template<typename SymbolType>
        bool is() const {
            return dynamic_cast<const SymbolType*>(this) != nullptr;
        }

        virtual ~Symbol() = default;

        virtual bool operator==(const Symbol &that) const {
            return id == that.id;
        }

        /** @brief stream operator */
        friend std::ostream &operator<<(std::ostream &os, const Symbol &that) {
            return os << that.id << "\t" << that.declarationLocations.front();
        }
    };

}


#endif //BRAINFUCKLANG_SYMBOL_H
