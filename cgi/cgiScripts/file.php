#!/usr/bin/php-cgi
<?php
// Output the required header first
echo "Content-Type: text/html\n\n";

echo "<html><body>\n";
echo "<h1>Hello from PHP CGI!</h1>\n";
echo "<p>Request method: " . getenv('REQUEST_METHOD') . "</p>\n";
echo "<p>Query string: " . getenv('QUERY_STRING') . "</p>\n";

// For POST data: read CONTENT_LENGTH bytes from stdin
if (getenv('REQUEST_METHOD') === 'POST') {
    $content_length = getenv('CONTENT_LENGTH');
    $post_data = file_get_contents("php://input", false, null, 0, $content_length);
    echo "<p>POST data: " . htmlspecialchars($post_data) . "</p>";
}

echo "</body></html>";
?>
