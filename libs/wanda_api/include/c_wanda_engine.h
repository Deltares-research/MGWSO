#ifndef C_WANDA_ENGINE
#define C_WANDA_ENGINE
#include <string>

extern "C"
{
    const char *wnd_eng_get_last_error();
    void *wnd_eng_get_instance(const char *wanda_bin);
    int wnd_eng_initialize(void *engine, void *model);
    int wnd_eng_run_steady(void *engine);
    int wnd_eng_run_time_step(void *engine);
    int wnd_eng_finish_unsteady(void *engine);
    int wnd_eng_close(void *engine);
    int wnd_eng_get_value(void *engine, char *name, char *prop, double *result);
    int wnd_eng_get_value_comp(void *engine, void *name, char *prop, double *result);
    int wnd_eng_set_value(void *engine, char *name, char *prop, double *value);
    int wnd_eng_set_value_comp(void *engine, void *name, char *prop, double *value);
    int wnd_eng_get_vector(void *engine, char *name, char *prop, double *values, int buffer_size);
    int wnd_eng_get_vector_comp(void *engine, void *comp, char *prop, double *values, int buffer_size);
    int wnd_eng_get_start_time(void *engine, double *time);
    int wnd_eng_get_end_time(void *engine, double *time);
    int wnd_eng_get_current_time(void *engine, double *time);
    int wnd_eng_get_time_step(void *engine, double *time);
}

#endif
