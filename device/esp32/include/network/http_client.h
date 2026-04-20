#pragma once

#include <stdbool.h>

bool http_client_post_json(const char *url, const char *payload, int expected_status_code);
bool http_client_post_json_with_status_range(
    const char *url,
    const char *payload,
    int min_status_code,
    int max_status_code
);
bool http_client_post(const char *url, int expected_status_code);
bool http_client_post_with_status_range(const char *url, int min_status_code, int max_status_code);
