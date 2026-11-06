<?php
declare(strict_types=1);

/**
 * Legacy unversioned text-command API permanently disabled.
 * Use diagnostics.php for authenticated, read-only ResoNet Protocol v1 access.
 */
header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-store');
header('X-Content-Type-Options: nosniff');
http_response_code(410);
echo json_encode([
    'success' => false,
    'error' => 'Legacy API disabled. Use diagnostics.php; protocol v1 is read-only.'
], JSON_UNESCAPED_UNICODE);
