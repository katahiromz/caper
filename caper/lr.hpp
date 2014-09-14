// Copyright (C) 2006 Naoyuki Hirayama.
// All Rights Reserved.

// $Id$

#ifndef LR_HPP
#define LR_HPP

#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "grammar.hpp"

namespace zw {

namespace gr {

/*============================================================================
 *
 * utility functions for marging sets
 *
 * セットのマージ用ユーティリティ関数
 *
 *==========================================================================*/

template < class D, class S > inline void merge_sets( D& x, const S& y ) { x.insert( y.begin(), y.end() ); }

/*============================================================================
 *
 * class core
 *
 * LR(0)項
 *
 *==========================================================================*/

template <class Token, class Traits>
class core {
public:
    typedef zw::gr::core<Token, Traits>           self_type;
    typedef zw::gr::rule<Token, Traits>           rule_type;
    typedef zw::gr::symbol<Token, Traits>        symbol_type;

public:
    core(const rule_type& r, int i) : rule_(r), cursor_(i) {}
    core(const self_type& x) : rule_(x.rule_), cursor_(x.cursor_) {}

    self_type& operator=(const self_type& x) {
        rule_ = x.rule_;
        cursor_ = x.cursor_;
        return *this;
    }

    int cmp(const core<Token, Traits>& y) const {
        return id() != y.id() ? (id() - y.id()) : (cursor() - y.cursor());
    }

    int                 id() const      { return rule_.id(); }
    const rule_type&    rule() const    { return rule_; }
    int                 cursor() const  { return cursor_; }

    const symbol_type&  curr() const    { return rule_.right()[cursor_]; }
    bool                over() const    {
        return int(rule_.right().size()) <= cursor_;
    }

private:
    rule_type   rule_;
    int         cursor_;

};

template <class Token, class Traits>
bool operator<(const core<Token, Traits>& x,
               const core<Token, Traits>& y) {
    return x.cmp(y) < 0;
}

template <class Token, class Traits>
bool operator==(const core<Token, Traits>& x,
                const core<Token, Traits>& y) {
    return x.cmp(y) == 0;
}

template <class Token, class Traits>
std::ostream& operator<<(std::ostream& os, const core<Token, Traits>& y) {
    typedef rule<Token, Traits> rule_type;

    const rule_type& r = y.rule();
    os << "r" << y.id() << ": " << r.left() << " :: = ";

    int n = 0;
    for (const auto& x: r.right()) {
        if (n++ == y.cursor()) { os << " _"; }
        os << " " << x;
    }
    if (y.cursor() == int(r.right().size())) { os << " _"; }
    return os;
}

/*============================================================================
 *
 * class item
 *
 * LR(1)項
 *
 *==========================================================================*/

template <class Token, class Traits>
class item {
public:
    typedef zw::gr::core<Token, Traits>     core_type;
    typedef zw::gr::item<Token, Traits>     self_type;
    typedef zw::gr::symbol<Token, Traits>   symbol_type;
    typedef zw::gr::terminal<Token, Traits> terminal_type;
    typedef zw::gr::rule<Token, Traits>     rule_type;

public:
    item(const rule_type& r, int c, const terminal_type& s)
        : core_(r, c), lookahead_(s) {}
    item(const core_type& x, const terminal_type& y)
        : core_(x), lookahead_(y) {}
    item(const self_type& x) : core_(x.core_), lookahead_(x.lookahead_) {}

    self_type& operator=(const self_type& x) {
        core_ = x.core_;
        lookahead_ = x.lookahead_;
        return *this;
    }

    const core_type&        core() const    { return core_; }
    const terminal_type& lookahead() const  { return lookahead_; }

    int                     id() const      { return core_.id(); }
    const rule_type&        rule() const    { return core_.rule(); }
    int                     cursor() const  { return core_.cursor(); }

    const symbol_type&      curr() const    { return core_.curr(); }
    bool                    over() const    { return core_.over(); }

    int cmp(const self_type& y) const {
        int x = core_.cmp(y.core_);
        if (x != 0) { return x; }
        return lookahead_.cmp(y.lookahead_);
    }

private:
    core_type       core_;
    terminal_type   lookahead_;

};

template <class Token, class Traits>
bool operator<(const item<Token, Traits>& x, const item<Token, Traits>& y) {
    return x.cmp(y) < 0;
}

template <class Token, class Traits>
bool operator ==(const item<Token, Traits>& x, const item<Token, Traits>& y) {
    return x.cmp(y) == 0;
}

template <class Token, class Traits>
std::ostream& operator<<(std::ostream& os, const item<Token, Traits>& y) {
    os << y.core() << " / " << y.lookahead();
    return os;
}

/*============================================================================
 *
 * class symbol_set
 *
 * シンボル集合
 *
 *==========================================================================*/

template <class Token, class Traits>
class symbol_set :
        public std::unordered_set<symbol<Token, Traits>,
                                  typename symbol<Token, Traits>::hash> {
};

template <class Token, class Traits>
std::ostream& operator<<(std::ostream& os, const symbol_set<Token, Traits>& y) {
    os << '{';
    for (const auto& x: y) {
        os << x << ", ";
    }
    os << '}';
    return os;
}

/*============================================================================
 *
 * class core_set
 *
 * LR(0)項集合
 *
 *==========================================================================*/

template <class Token,class Traits >
class core_set : public std::set<core<Token, Traits>> {
};

template <class Token, class Traits>
std::ostream& operator<<(std::ostream& os, const core_set<Token, Traits>& s) {
    os << '{';
    for (const auto& x: s) {
        std::cerr << x << "; ";
    }
    os << '}';
    return os;
}

/*============================================================================
 *
 * class item_set
 *
 * LR(1)項集合
 *
 *==========================================================================*/

template <class Token, class Traits>
class item_set : public std::set<item<Token, Traits>> {
};

template <class Token, class Traits>
std::ostream& operator<<(std::ostream& os, const item_set<Token, Traits>& s) {
    os << '{';
    for (const auto& x: s) {
        os << x << "; ";
    }
    os << '}';
    return os;
}

/*============================================================================
 *
 * class first_collection
 *
 * FIRST(a)のコレクション
 *
 *==========================================================================*/

template <class Token, class Traits>
class first_collection :
        public std::unordered_map<symbol<Token, Traits>,
                                  symbol_set<Token, Traits>,
                                  typename symbol<Token, Traits>::hash> {
};

template <class Token, class Traits>
std::ostream& operator<<(
    std::ostream& os, const first_collection<Token, Traits>& s) {
    os << "{\n";
    for (const auto& x: s) {
        os << "    " << x.first << " = " << x.second << "; " << std::endl;
    }
    os << "}\n";
    return os;
}

/*============================================================================
 *
 * class follow_collection
 *
 * FOLLOW(a)のコレクション
 *
 *==========================================================================*/

template <class Token, class Traits>
class follow_collection :
        public std::unordered_map<symbol<Token, Traits>,
                                  symbol_set<Token, Traits>,
                                  typename symbol<Token, Traits>::hash> {
};

template <class Token, class Traits>
std::ostream& operator<<(
    std::ostream& os, const follow_collection<Token, Traits>& s) {
    os << "{\n";
    for (const auto& x: s) {
        os << "    " << x.first << " = " << x.second << "; " << std::endl;
    }
    os << "}\n";
    return os;
}

/*============================================================================
 *
 * class lr0_collection
 *
 * LR(0)集
 *
 *==========================================================================*/

template <class Token, class Traits>
class lr0_collection : public std::set<core_set<Token, Traits>> {
};

template <class Token, class Traits>
std::ostream& operator<<(
    std::ostream& os, const lr0_collection<Token, Traits>& C) {
    for (const auto& x: C) {
        os << x << std::endl;
    }
    return os;
}

/*============================================================================
 *
 * class lr1_collection
 *
 * LR(1)集
 *
 *==========================================================================*/

/*
template <class Token, class Traits>
class lr1_collection : public std::set<item_set<Token, Traits>> {
  public:
    lr1_collection() {}
    ~lr1_collection() {}
};

template <class Token, class Traits>
std::ostream& operator<<(std::ostream& os, const lr1_collection<Token, Traits>& C) {
    for (typename lr1_collection<Token, Traits>::const_iterator i = C.begin(); i != C.end(); ++i) {
        os <<(*i)<< std::endl;
    }
    return os;
}
*/

/*============================================================================
 *
 * collect_symbols
 *
 *
 *
 *==========================================================================*/

template <class Token, class Traits>
void collect_symbols(
    symbol_set<Token, Traits>&      terminals,
    symbol_set<Token, Traits>&      nonterminals,
    symbol_set<Token, Traits>&      all_symbols,
    const grammar<Token, Traits>&   g) {

    for (const auto& rule: g) {
        nonterminals.insert(rule.left());
        all_symbols.insert(rule.left());
        for (const auto& x: rule.right()) {
            if (x.is_terminal()) { terminals.insert(x); }
            if (x.is_nonterminal()) { nonterminals.insert(x); }
            all_symbols.insert(x); 
        }
    }
}

/*============================================================================
 *
 * make_first_and_follow
 *
 *
 *
 *==========================================================================*/

template <class Token, class Traits>
bool all_nullable(
    const std::unordered_set<symbol<Token, Traits>, typename symbol<Token, Traits>::hash>& nullable,
    const std::vector<symbol<Token, Traits>>&    rule_right,
    int b, int e) {
    bool flag = true;
    for (int j = b ; j <e ; j++) {
        const auto& s = rule_right[j];
        if (s.is_epsilon()) { continue; }

        if (nullable.count(s) == 0) {
            flag = false;
            break;
        }
    }
    return flag;
}

template <class Token, class Traits>
void make_first_and_follow(
    first_collection<Token, Traits>&    first,
    follow_collection<Token, Traits>&   follow,
    const symbol_set<Token, Traits>&    terminals,
    const symbol_set<Token, Traits>&,
    const symbol_set<Token, Traits>&,
    const grammar<Token, Traits>&       g) {
    typedef symbol_set<Token, Traits>     symbol_set_type;

    // TODO: followバグってるっぽい(使ってない)

    // nullable
    symbol_set_type nullable;

    // For each terminal symbol Z, FIRST[Z] = { Z }.
    for (const auto& x: terminals) {
        first[x].insert(x);
    }

    // repeat until FIRST, FOLLOW,
    // and nullable did not change in this iteration.
    bool repeat = true;
    while (repeat) {
        repeat = false;

        // for each production X -> Y1Y2...Yk
        for (const auto& rule: g) {
            // if Y1...Yk are all nullable(or if k = 0)
            int k = int(rule.right().size());
            if (all_nullable(nullable, rule.right(), 0, k)) {
                // nullable[X] = true
                if (!nullable.count(rule.left())) {
                    repeat = true; 
                    nullable.insert(rule.left());
                }
            }

            // for each i from 1 to k, each j from i + 1 to k
            for (int i = 0 ; i < k ; i++) {
                // if Y1...Yi-1 are all nullable(or if i = 1)
                if (all_nullable(nullable, rule.right(), 0, i)) {
                    // then FIRST[X] = FIRST[X] unify FIRST[Yi]
                    symbol_set_type s = first[rule.left()];
                    merge_sets(s, first[rule.right()[i]]);

                    if (first[rule.left()] != s) {
                        repeat = true;
                        first[rule.left()] = s;
                    }
                }
                // if Yi+1...Yk are all nullable(or if i = k)
                if (all_nullable(nullable, rule.right(), i+1, k)) {
                    // then FOLLOW[Yi] = FOLLOW[Yi] unify FOLLOW[X]
                    symbol_set_type s = follow[rule.right()[i]];
                    merge_sets(s, follow[rule.left()]);

                    if (follow[rule.right()[i]] != s) {
                        repeat = true;
                        follow[rule.right()[i]] = s;
                    }
                }

                for (int j = i+1 ; j < k ; j++) {
                    // if Yi+1...Yj-1 are all nullable(or if i+1 = j)
                    // then FOLLOW[Yi] = FOLLOW[Yi] unify FIRST[Yj]
                    if (all_nullable(nullable, rule.right(), i+1, j)) {
                        symbol_set_type s = follow[rule.right()[i]];
                        merge_sets(s, first[rule.right()[j]]);

                        if (follow[rule.right()[i]] != s) {
                            repeat = true;
                            follow[rule.right()[i]] = s;
                        }
                    }
                }
            }

        }
    }

    for (const auto& x: nullable) {
        first[x].insert(epsilon<Token, Traits>());
    }
}

template <class Token, class Traits>
void make_vector_first(
    terminal_set<Token, Traits>&                s,
    const first_collection<Token, Traits>&      first,
    const std::vector<symbol<Token, Traits>>&   v) {

    // n番目の要素にepsilonが含まれている場合、
    // n+1番目の要素も追加する

    bool next = false;
    for (const auto& x: v) {
        next = false;

        auto j = first.find(x);
        assert(j != first.end());

        for (const auto& k: (*j).second) {
            assert(!k.is_nonterminal());
            if (k.is_epsilon()) {
                next = true;
            } else {
                s.insert(k.as_terminal());
            }
        }
        if (!next) { break; }
    }
}

/*============================================================================
 *
 * make_lr0_closure
 *
 * LR(0)closureの作成
 *
 *==========================================================================*/

template <class Token, class Traits>
void
make_lr0_closure(
    core_set<Token, Traits>&      J,
    const grammar<Token, Traits>& g) {
    typedef symbol<Token, Traits>         symbol_type;
    typedef rule<Token, Traits>           rule_type;
    typedef core<Token, Traits>           core_type;
    typedef core_set<Token, Traits>       core_set_type;

    std::unordered_set<const std::string*> added;

    size_t J_size;
    do {
        core_set_type new_cores;

        J_size = J.size();

        for (const core_type& x: J) {
            if (x.over()) { continue; }

            const symbol_type& y = x.curr();
            if (!y.is_nonterminal()) { continue; }
            if (added.find(y.identity()) != added.end()) { continue; }

            for (const rule_type& z:
                     (*g.dictionary().find(y.identity())).second) {
                new_cores.insert(core_type(z, 0)); 
            }
            added.insert(y.identity());
        }

        J.insert(new_cores.begin(), new_cores.end());
    } while (J_size != J.size());
}

/*============================================================================
 *
 * make_lr0_goto
 *
 * LR(0)gotoの作成
 *
 *==========================================================================*/

template <class Token, class Traits>
void make_lr0_goto(
    core_set<Token, Traits>&              J,
    const core_set<Token, Traits>&        I,
    const symbol<Token, Traits>&          X,
    const grammar<Token, Traits>&         g) {
    typedef symbol<Token, Traits> symbol_type;
    typedef core<Token, Traits>   core_type;

    for (const core_type& x: I) {
        if (x.over()) { continue; }

        const symbol_type& y = x.curr(); 
        if (!(y == X)) { continue; }

        J.insert(core_type(x.rule(), x.cursor() + 1)); 
    }

    make_lr0_closure(J, g);
}

/*============================================================================
 *
 * make_lr1_closure
 *
 * LR(1)closureの作成
 *
 *==========================================================================*/

template <class Token, class Traits>
void
make_lr1_closure(
    item_set<Token, Traits>&                      J,
    const first_collection<Token, Traits>&        first,
    const grammar<Token, Traits>&                 g) {

    typedef symbol<Token, Traits>               symbol_type;
    typedef terminal<Token, Traits>             terminal_type;
    typedef rule<Token, Traits>                 rule_type;
    typedef item<Token, Traits>                 item_type;
    typedef symbol_set<Token, Traits>           symbol_set_type;
    typedef terminal_set<Token, Traits>         terminal_set_type;
    typedef item_set<Token, Traits>             item_set_type;
    typedef std::vector<symbol<Token, Traits>>  symbol_vector_type;

    size_t J_size;
    do {
        item_set_type new_items;  // 挿入する項

        J_size = J.size();

        for (const item_type& x: J) {
            // x is [item(A→α・Bβ, a)]
            if (x.over()) { continue; }

            // y is [symbol(B)]
            const symbol_type& y = x.curr();
            if (!y.is_nonterminal()) { continue; }

            // v is [symbol_vector_type(βa)]
            symbol_vector_type v;
            const auto& right = x.rule().right();
            v.assign(right.begin()+ x.cursor()+ 1, right.end());
            v.push_back(x.lookahead());

            // f is FIRST(βa)
            terminal_set_type f;
            make_vector_first(f, first, v); 

            for (const rule_type& z:
                     (*g.dictionary().find(y.identity())).second) {
                // z is [rule(B→γ)]

                // 各lookahead
                for (const terminal_type& s: f) {
                    new_items.insert(item_type(z, 0, s));
                }
            }
        }

        merge_sets(J, new_items);
    } while (J_size != J.size());
}

/*============================================================================
 *
 * make_lr1_goto
 *
 * LR(1)gotoの作成
 *
 *==========================================================================*/

template <class Token, class Traits>
void
make_lr1_goto(
    item_set<Token, Traits>&                      J,
    const item_set<Token, Traits>&                I,
    const symbol<Token, Traits>&                  X,
    const first_collection<Token, Traits>&        first,
    const grammar<Token, Traits>&                 g) {
    typedef symbol<Token, Traits>         symbol_type;
    typedef item<Token, Traits>           item_type;

    for (const item_type& x: I) {
        if (x.over()) { continue; }

        const symbol_type& y = x.curr();
        if (!(y == X)) { continue; }

        J.insert(item_type(x.rule(), x.cursor()+1, x.lookahead()));
    }

    make_lr1_closure(J, first, g);
}

/*============================================================================
 *
 * make_lr0_collection
 *
 * 正準LR(0)集の作成
 *
 *==========================================================================*/

template <class Token, class Traits>
void
make_lr0_collection(
    lr0_collection<Token, Traits>&        C,
    const grammar<Token, Traits>&         g) {
    typedef symbol<Token, Traits>           symbol_type;
    typedef core<Token, Traits>             core_type; 
    typedef lr0_collection<Token, Traits>   lr0_collection_type; 
    typedef symbol_set<Token, Traits>       symbol_set_type; 
    typedef core_set<Token, Traits>         core_set_type; 

    // 記号の収集
    symbol_set_type syms;
    for (const auto& r: g) {
        syms.insert(r.left());
        syms.insert(r.right().begin(), r.right().end());
    }

    // 正準集の作成
    core_set_type s;
    s.insert(core_type(g.root_rule(), 0));
    make_lr0_closure(s, g);
    C.insert(s);

    size_t C_size;
    do {
        lr0_collection_type new_collection; // 挿入する項集合

        C_size = C.size();

        for (const core_set_type& I: C) {
            for (const symbol_type& X: syms) {
                core_set_type I_dash;
                make_lr0_goto(I_dash, I, X, g);

                if (!I_dash.empty()) {
                    new_collection.insert(std::move(I_dash));
                }
            }
        }

        for (auto&& c: new_collection) {
            C.insert(c);
        }
    } while (C_size != C.size());
}

/*============================================================================
 *
 * choose_kernel
 *
 * LR(0)項集合から主要素項を求める
 *
 *==========================================================================*/

template <class Token, class Traits>
void
choose_kernel(
    core_set<Token, Traits>&       K,
    const core_set<Token, Traits>& I,
    const grammar<Token, Traits>& g) {

    for (const auto& x: I) {
        if (x.rule() == g.root_rule() || 0 < x.cursor()) {
            K.insert(x);
        }
    }
}

/*============================================================================
 *
 * items_to_cores
 *
 * 
 *
 *==========================================================================*/

template <class Token, class Traits>
void
items_to_cores(
    core_set<Token, Traits> &        xx,
    const item_set<Token, Traits>&   x) {

    for (const auto& y: x) {
        xx.insert(y.core());
    }
}

/*============================================================================
 *
 * class parsing_table
 *
 * 解析表
 *
 *==========================================================================*/

enum action_t {
    action_shift,
    action_reduce,
    action_accept,
    action_error,
};

template <class Token, class Traits>
class parsing_table {
public:
    struct state;

    typedef Token                       token_type;
    typedef Traits                      traits_type;
    typedef parsing_table<Token,Traits> self_type;
    typedef symbol<Token, Traits>       symbol_type;
    typedef symbol_set<Token, Traits>   symbol_set_type;
    typedef terminal_set<Token, Traits> terminal_set_type;
    typedef grammar<Token, Traits>      grammar_type;
    typedef rule<Token, Traits>         rule_type;
    typedef core<Token, Traits>         core_type; 
    typedef item<Token, Traits>         item_type; 
    
    struct action {
        action_t type;

        int         dest_index; // index to states_
        rule_type   rule;

        action(){}
        action(action_t at, int di, const rule_type& r)
            : type(at), dest_index(di), rule(r) {}
    };

    struct state {
    public:
        typedef item_set<Token, Traits>                 item_set_type;
        typedef core_set<Token, Traits>                 core_set_type;
        typedef std::map<Token, action>                 action_table_type;
        typedef std::map<symbol_type, int>              goto_table_type; // index to states_
        typedef std::map<core_type, terminal_set_type>  generate_map_type;
        typedef std::set<std::pair<int, core_type>>     propagate_type;
        typedef std::map<core_type, propagate_type>     propagate_map_type;

        int                     no              = -1;
        core_set_type           cores;
        core_set_type           kernel;
        item_set_type           items;
        generate_map_type       generate_map;
        propagate_map_type      propagate_map;

        goto_table_type         goto_table;
        action_table_type       action_table;
        bool                    handle_error    = false;

        state(int n) : no(n) {}
    };

    typedef std::vector<state> states_type;

public:
    parsing_table() { first_ = -1; }
    parsing_table(const parsing_table<Token, Traits>& x) { operator=(x); }
    ~parsing_table() { clear(); }

    self_type& operator=(const self_type& x) {
        clear();
        states_ = x.states_;
        grammar_ = x.grammar_;
        first_ = x.first_;
        return *this;
    }

    void set_grammar(const grammar_type& g) { grammar_ = g; }

    int     first_state() const { return first_; }

    const states_type& states() const { return states_; }
    const grammar_type& get_grammar() const { return grammar_; }

    void first_state(int s) {
        enunique();
        first_ = s;
    }

    states_type& states() {
        enunique();
        return states_;
    }

    state& add_state() {
        enunique();
        states_.emplace_back(int(states_.size()));
        return states_.back();
    }

protected:
    void clear() {
        states_.clear();
        grammar_ = grammar_type();
    }

    void enunique(){}

private:
    states_type     states_;
    grammar_type    grammar_;
    int             first_;
    
};

template <class Token, class Traits>
std::ostream& operator<<(
    std::ostream& os, const parsing_table<Token, Traits>& x) {

    os << "<toplevel = state" << x.first_state() << ">\n";
    for (const auto& state: x.states()) {
        os << "<state: " << state.no << ">\n";
        for (const auto& pair: state.action_table) {
            os << "  action(";
            if (pair.first == Traits::eof()) {
                os << "eof";
            } else {
                os << pair.first;
            }
            os << ") = ";

            const auto& action = pair.second;

            switch (action.type) {
                case action_shift:
                    os << "shift(" << action.dest_index << ")\n";
                    break;
                case action_reduce:    
                    os << "reduce(" << action.rule << ")\n";
                    break;
                case action_accept:
                    os << "accept(" << action.rule << ")\n";
                    break;
                case action_error:
                    os << "error\n";
                    break;
            }
        }
        for (const auto& pair: state.goto_table) {
            os << "  goto(" << pair.first << ") = ";
            os << pair.second << "\n";
        }
    }
    return os;
}

template <class Token, class Traits>
struct null_reporter {
    typedef rule<Token, Traits> rule_type;

    void operator()(const rule_type&, const rule_type& y) {
        // do nothing
    }
};

} // namespace gr

} // namespace zw

#endif // LR_HPP
