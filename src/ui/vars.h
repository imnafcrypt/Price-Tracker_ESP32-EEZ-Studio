#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_PAIR = 0,
    FLOW_GLOBAL_VARIABLE_PRICE = 1,
    FLOW_GLOBAL_VARIABLE_BUY = 2,
    FLOW_GLOBAL_VARIABLE_SELL = 3,
    FLOW_GLOBAL_VARIABLE_HIGH = 4,
    FLOW_GLOBAL_VARIABLE_LOW = 5,
    FLOW_GLOBAL_VARIABLE_VOLBASE = 6,
    FLOW_GLOBAL_VARIABLE_VOLQUOTE = 7,
    FLOW_GLOBAL_VARIABLE_NETWORK = 8,
    FLOW_GLOBAL_VARIABLE_BASE = 9,
    FLOW_GLOBAL_VARIABLE_QUOTE = 10
};

// Native global variables

extern const char *get_var_pair();
extern void set_var_pair(const char *value);
extern const char *get_var_price();
extern void set_var_price(const char *value);
extern const char *get_var_buy();
extern void set_var_buy(const char *value);
extern const char *get_var_sell();
extern void set_var_sell(const char *value);
extern const char *get_var_high();
extern void set_var_high(const char *value);
extern const char *get_var_low();
extern void set_var_low(const char *value);
extern const char *get_var_volbase();
extern void set_var_volbase(const char *value);
extern const char *get_var_volquote();
extern void set_var_volquote(const char *value);
extern bool get_var_network();
extern void set_var_network(bool value);
extern const char *get_var_base();
extern void set_var_base(const char *value);
extern const char *get_var_quote();
extern void set_var_quote(const char *value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/