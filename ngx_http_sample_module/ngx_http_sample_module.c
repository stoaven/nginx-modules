#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_flag_t  sample_enable;
} ngx_http_sample_loc_conf_t;

typedef struct {
	ngx_str_t sample_ids;
} ngx_http_sample_ctx_t;

static ngx_int_t ngx_http_sample_init(ngx_conf_t *cf);
static void *ngx_http_sample_create_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_sample_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_sample_add_variables(ngx_conf_t *cf);
static ngx_int_t ngx_http_sample_ids_variable(ngx_http_request_t *r, 
    ngx_http_variable_value_t *v, uintptr_t data);


static ngx_command_t ngx_http_sample_commands[] = {

    {   ngx_string("sample_enable"),
        NGX_HTTP_LOC_CONF|NGX_CONF_FLAG, 
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_sample_loc_conf_t, sample_enable),
        NULL },

    ngx_null_command
};

static ngx_http_module_t ngx_http_sample_module_ctx = {
    
    ngx_http_sample_add_variables,      /* preconfiguration */
    ngx_http_sample_init,                /* postconfiguration */

    NULL,                                   /* create main configuration, http {}, 1 */
    NULL,                                   /* init main configuration, 4 */

    NULL,                                   /* create server configuration, server {}, 2 */
    NULL,                                   /* merge server configuration, 5 */

    ngx_http_sample_create_loc_conf,    /* create location configuration, location {}, 3 */
    NULL                                    /* merge location configuration, 6 */
};

ngx_module_t ngx_http_sample_module = {
    
    NGX_MODULE_V1,
    &ngx_http_sample_module_ctx,    /* module context, point to ngx_http_module_t */
    ngx_http_sample_commands,       /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                           /* init master */
    NULL,                           /* init module, no use in http module*/
    NULL,                           /* init process, no use */
    NULL,                           /* init thread */
    NULL,                           /* exit thread */
    NULL,                           /* exit process, no use */
    NULL,                           /* exit master, no use */
    NGX_MODULE_V1_PADDING
};

static ngx_http_variable_t ngx_http_sample_variables[] = {
    
    {   ngx_string("sample_ids"), NULL, 
        ngx_http_sample_ids_variable, 0, 
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    {   ngx_null_string, NULL, NULL, 0, 0, 0 }
};


static ngx_int_t
ngx_http_sample_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_sample_handler;

    return NGX_OK;
}


static ngx_int_t 
ngx_http_sample_handler(ngx_http_request_t *r) 
{
    ngx_http_sample_loc_conf_t *slcf = NULL;
	ngx_http_sample_ctx_t  *ctx = NULL;
	ngx_str_t sample_ids = ngx_string("456_789_123");
	

    slcf = ngx_http_get_module_loc_conf(r, ngx_http_sample_module);
	if (slcf->sample_enable == NGX_CONF_UNSET) {
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "sample module is not enable!");
        return NGX_DECLINED;
	}

    ctx = ngx_http_get_module_ctx(r, ngx_http_sample_module);

    if (ctx == NULL) {
		ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_sample_ctx_t));
		if (ctx == NULL) {
			return NGX_ERROR;
		}
		ngx_http_set_ctx(r, ctx, ngx_http_sample_module);
    }

	ctx->sample_ids.data = ngx_palloc(r->pool, sample_ids.len);
	if (ctx->sample_ids.data == NULL) {
		return NGX_ERROR;
	}
	
	ngx_memcpy(ctx->sample_ids.data, sample_ids.data, sample_ids.len);
	ctx->sample_ids.data.len = sample_ids.len;
	
    return NGX_DECLINED;
}


static void *
ngx_http_sample_create_loc_conf(ngx_conf_t *cf) 
{
    ngx_http_sample_loc_conf_t* slcf = NULL;
    
    slcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_sample_loc_conf_t));
    if (slcf == NULL) {
        return NULL;
    }

    slcf->sample_enable = NGX_CONF_UNSET;
    
    return slcf;
}


static ngx_int_t
ngx_http_sample_add_variables(ngx_conf_t *cf) 
{
    ngx_http_variable_t     *var, *v;

	
    for (v = ngx_http_sample_variables; v->name.len; v++) {

        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}


static ngx_int_t 
ngx_http_sample_ids_variable(ngx_http_request_t *r, 
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_sample_ctx_t  *ctx = NULL;


    ctx = ngx_http_get_module_ctx(r, ngx_http_sample_module);

    if (ctx == NULL) {
		v->not_found = 1;
		return NGX_OK;
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    v->data = ctx->sample_ids.data;
    v->len = ctx->sample_ids.len;

    return NGX_OK;
}


