//-----------------------------------------------------------------------------------------
// Copyright (C) 2026 Jeremy Lorelli
//-----------------------------------------------------------------------------------------
// Purpose: Parser for ADL files. ADL is a pretty simple structured format used by the MEDM
//  display software (along with other, older softwares, I think).
//  This can be used to parse generic structured files in that format.
//-----------------------------------------------------------------------------------------
// This file is part of 'c-utils'. It is subject to the license terms in the
// LICENSE file found in the top-level directory of this distribution.
// No part of 'c-utils', including this file, may be copied, modified, propagated,
// or otherwise distributed except according to the terms contained in the LICENSE file.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus

#include <list>
#include <string>
#include <variant>
#include <vector>
#include <cctype>

/**
 * Represents a value in an ADL file, which may be the root node (in which case it's the whole file).
 * ADL itself is a pretty simple structured tree of elements. An ADL value is either a block
 * or a value.
 * 'Blocks' have 0 or more children, a name and no value.
 * 'Blocks' may also be arrays if their children have no names (value-only), however no distinction
 * is made in the API between blocks and arrays.
 * 'Values' always have a value set. They may have no name if they're array values, in which case
 * their parent is the containing array.
 * 'Values' may not have any children, since there is no way to represent this in the ADL syntax.
 */
class adl_node {
protected:
    std::string m_name;
    std::string m_value;
    std::vector<adl_node*> m_children;

public:
    adl_node() = delete;
    
    /**
     * @brief Create a new ADL node.
     * @param name Name of the node. Leave empty and set value to create an array entry.
     * @param value Value. This should be left empty for data blocks.
     */
    inline adl_node(const std::string& name, const std::string& value = {}) :
        m_name(name),
        m_value(value)
    {
    }
    
    inline ~adl_node()
    {
        clear_children();
    }

    /**
     * @brief Clears children nodes, deleting them after being cleared.
     * This node can be reused as a block or value node after this operation.
     */
    inline void clear_children()
    {
        for (auto& v : m_children) {
            delete v;
        }
        m_children.clear();
    }
    
    /**
     * @brief Sets a value on this node.
     * @param v The new value.
     * @param force If true, forcibly converts the node to a value type instead of failing.
     * @returns true on success, false if this node was a value type and 'force' is false.
     */
    inline bool set_value(const std::string& v, bool force = false)
    {
        if (m_children.size()) {
            if (!force)
                return false;
            clear_children();
        }
        m_value = v;
        return true;
    }

    inline const std::string& value() const { return m_value; }

    /**
     * @brief Adds a child to this value node.
     * If this node is a 'value', it will be implicitly converted to a block.
     * @param val The ADL value to add
     */
    inline void add_child(adl_node* val)
    {
        m_children.push_back(val);
        m_value.clear(); /* blocks cannot have a value */
    }

    /**
     * @brief Sets the name of the value.
     * Note that this implicitly makes this a non-array item, unless n = {}
     * @param n Name of the node. If empty, effectively converts this to an array item.
     */
    inline void set_name(const std::string& n) { m_name = n; }

    inline const std::string& name() const { return m_name; }

    inline bool is_value() const { return !m_value.empty(); }
    inline bool is_block() const { return m_value.empty(); }
    inline bool is_array_item() const { return m_name.empty() && is_value(); }

    inline size_t child_count() const { return m_children.size(); }

    /**
     * @brief Return the child at the specified index.
     * @param index The index
     * @returns Pointer to the child. null if the index is out of range or if this is not a block.
     */
    inline adl_node* child(size_t index) const
    {
        if (index >= child_count() || !is_block())
            return nullptr;
        return m_children[index];
    }
    
    /**
     * @brief Looks up the child based on the name.
     * This is a linear O(n) lookup.
     * @param name The name to search for.
     * @returns Pointer to the child, or nullptr if not found.
     */
    inline adl_node* child(const std::string& name) const
    {
        for (auto& k : m_children) {
            if (k->name() == name)
                return k;
        }
        return nullptr;
    }

    /**
     * @brief Dump the ADL value and its children to the specified stream
     * @param indent_amount The number of spaces to indent by per level.
     * @param indent_level The indent level to use. 0 should be the start.
     */
    inline void dump(FILE* fp, int indent_amount = 4, int indent_level = 0)
    {
        if (!m_value.empty()) {
            if (m_name.empty()) { /* array values have no name */
                const bool vq = needs_quotes(m_value);
                fprintf(
                    fp, "%*s%s%s%s\n", indent_level * indent_amount, "",
                    vq ? "\"" : "", m_value.c_str(), vq ? "\"" : ""
                );
            } else {
                const bool vq = needs_quotes(m_value);
                const bool nq = needs_quotes(m_name);
                fprintf(
                    fp, "%*s%s%s%s=%s%s%s\n", indent_level * indent_amount, "",
                    nq ? "\"" : "", m_name.c_str(), nq ? "\"" : "",
                    vq ? "\"" : "", m_value.c_str(), vq ? "\"" : ""
                );
            }
        } else {
            if (!m_name.empty()) { /* Root level has no name */
                const bool nq = needs_quotes(m_name);
                fprintf(
                    fp, "%*s%s%s%s {\n", indent_level * indent_amount, "",
                    nq ? "\"" : "", m_name.c_str(), nq ? "\"" : ""
                );
            }

            for (auto& v : m_children) {
                /* avoid indenting root */
                v->dump(fp, indent_amount, m_name.empty() ? 0 : indent_level+1);
            }

            if (!m_name.empty()) {
                fprintf(fp, "%*s}\n", indent_level * indent_amount, "");
            }
        }
    }

    /**
     * @brief Parse an ADL file out of the specified NULL terminated buffer.
     * @param buffer Pointer to the buffer
     * @param errors Error buffer
     * @returns adl_value* on success
     */
    static adl_node* parse(const char* buffer, std::vector<std::string>* errors = nullptr) {
        std::vector<adl_node*> stack;
        std::string token, value;

        auto* root = new adl_node("");
        stack.push_back(root);

        auto skipWs = [](const char*& s, int& ln) {
            while (*s && std::isspace(*s)) {
                if (*s == '\n')
                    ln++;
                s++;
            }
        };

        int ln = 0;
        bool errored = false;

        for (const char* s = buffer; *s;) {
            skipWs(s, ln);
            if (!*s)
                break;

            consumeToken(s, token);
            skipWs(s, ln);
            if (!*s)
                break;

            /* check next tokens */
            if (*s == '{') { /* Opening a new block */
                auto* block = new adl_node(token);
                stack.back()->add_child(block);
                stack.push_back(block);
                token.clear();
                ++s;
                continue;
            } else if (*s == '=') { /* Item assignment */
                ++s; /* skip operator */
                consumeToken(s, value);
                stack.back()->add_child(new adl_node(token, value));
                token.clear();
                value.clear();
                ++s;
                continue;
            } else if (*s == ',') { /* array item */
                ++s; /* skip operator */
                stack.back()->add_child(new adl_node({}, value));
                token.clear();
            } else if (token == "}") { /* closing block (current token) */
                if (stack.empty()) {
                    errored = true;
                    if (errors) {
                        char buf[128];
                        snprintf(buf, sizeof(buf), "Line %d: Unmatched brace\n", ln);
                        errors->push_back(buf);
                    }
                    break;
                }
                stack.pop_back();
                token.clear();
            } else {
                if (errors) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Line %d: Unknown token '%c'\n", ln, *s);
                    errors->push_back(buf);
                }
                errored = true;
                break;
            }
        }
        
        if (stack.size() > 1) {
            if (errors) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Line %d: Unterminated block\n", ln);
                errors->push_back(buf);
            }
        }
        
        if (errored) {
            delete root;
            return nullptr;
        }

        return root;
    }
    
private:
    inline bool is_ctrl(char c)
    {
        switch (c) {
        case '=':
        case ',':
        case '{':
        case '}':
        case '$': /* $() are not control chars, but for stylistic reasons let's keep these quoted */
        case ')':
        case '(':
            return true;
        default:
            return false;
        }
    }

    /**
     * @brief Check if a string needs quotes.
     */
    inline bool needs_quotes(const std::string& s)
    {
        for (auto& k : s)
            if (std::isspace(k) || is_ctrl(k))
                return true;
        return false;
    }
    
    /**
     * @brief Consume a token. Assumes 's' is already aligned to a non-ws char
     */
    static void consumeToken(const char*& s, std::string& tokenbuf)
    {
        /* Handle quoted tokens */
        if (*s == '"') {
            ++s; /* skip leading quote */
            for (; *s; ++s) {
                if (*s == '\"') {
                    ++s; /* Skip final quote */
                    return;
                } else if (*s == '\\' && *(s+1) == '"') {
                    tokenbuf.push_back(*s);
                    ++s; /* skip next quote */
                }
                tokenbuf.push_back(*s);
            }
            return;
        }

        /* Non quoted tokens */
        while (*s && !std::isspace(*s) && *s != '=' && *s != ',') {
            tokenbuf.push_back(*s);
            ++s;
        }
    }
};


#endif // __cplusplus