#pragma once

#include <stdbool.h>
#include <stddef.h>


bool http_client_post_json(
    const char *url, 
    const char *payload, 
    int expected_status_code
);
bool http_client_post_json_with_status_range(
    const char *url,
    const char *payload,
    int min_status_code,
    int max_status_code
);
bool http_client_post_json_with_response_and_status_range(
    const char *url,
    const char *payload,
    char *response_buffer,
    size_t buffer_size,
    int min_status_code,
    int max_status_code
);

bool http_client_post(
    const char *url, 
    int expected_status_code
);
bool http_client_post_with_status_range(
    const char *url, 
    int min_status_code, 
    int max_status_code
);

bool http_client_post_with_response(
    const char *url, 
    char *response_buffer, 
    size_t buffer_size, 
    int expected_status_code
);
bool http_client_post_with_response_and_status_range(
    const char *url, 
    char *response_buffer, 
    size_t buffer_size, 
    int min_status_code, 
    int max_status_code
);
