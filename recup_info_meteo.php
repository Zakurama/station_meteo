<?php

include 'private/config.php';

// Set error reporting for debugging (optional for development)
error_reporting(E_ALL);
ini_set('display_errors', 1);

// Set the content type to JSON
header('Content-Type: application/json');

// Check if the request method is POST
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Get the raw POST data
    $rawData = file_get_contents('php://input');

    // Decode the JSON data into a PHP associative array
    $data = json_decode($rawData, true);

    // Check if the required keys are present
    if (isset($data['temperature'], $data['pression'], $data['humidite'], $data['password']) && $data['password'] === $password) {
        // Extract values
        $temperature = $data['temperature'];
        $pression = $data['pression'];
        $humidite = $data['humidite'];

        $dbpath = 'private/info_meteo.db';
        // Create (or open) the SQLite database
        $db = new SQLite3($dbpath);

        // Create the table if it doesn't exist
        $db->exec('CREATE TABLE IF NOT EXISTS meteo (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, temperature REAL, humidite REAL, pression REAL)');

        // Calculate the threshold timestamp for the last day
        $threshold = time() - 86400; // 24 hours ago

        // Delete rows older than the last day
        $query = 'DELETE FROM meteo WHERE timestamp < :threshold';
        $stmt = $db->prepare($query);
        $stmt->bindValue(':threshold', $threshold, SQLITE3_INTEGER);
        $stmt->execute();

        $current_time = time();
        // Ensure there's only one row by replacing the existing row
        $stmt = $db->prepare('INSERT INTO meteo (timestamp, temperature, humidite, pression) VALUES (:current_time, :temperature, :humidite, :pression)');
        $stmt->bindValue(':current_time', $current_time, SQLITE3_INTEGER);
        $stmt->bindValue(':temperature', $temperature, SQLITE3_FLOAT);
        $stmt->bindValue(':humidite', $humidite, SQLITE3_FLOAT);
        $stmt->bindValue(':pression', $pression, SQLITE3_FLOAT);

        // Execute the statement
        $stmt->execute();

        // Close the database connection
        $db->close();
        $response = ['status' => 'success'];
        echo json_encode($response);
        exit();
    } else {
        $response = ['status' => 'error'];
        header("HTTP/1.1 400 Bad Request");
        echo json_encode($response);
        exit();
    }
} else {
    // Handle incorrect request method
    $response = ['status' => 'error'];
    header("HTTP/1.1 405 Method Not Allowed");
    echo json_encode($response);
    exit();
}
