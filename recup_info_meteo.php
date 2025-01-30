<?php

$day_in_second = 86400;

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

    if (isset($data['password'], $data['data']) && is_array($data['data'])){
        if (!password_verify($data['password'], $hashed_password)) {
            $response = ['status' => 'wrong password'];
            header("HTTP/1.1 400 Bad Request");
            echo json_encode($response);
            exit();
        }

        // Sort the data so that the last element (most recent) is first
        usort($data['data'], function ($a, $b) {
            return $b['timestamp'] - $a['timestamp']; // Sort descending by timestamp
        });

        $first_timestamp = $data['data'][0]['timestamp'];

        // timestep relative to the biggest one (biggest one put at 0)
        foreach ($data['data'] as &$item) {
            $item['timestamp'] -= $first_timestamp;
        }
        unset($item);
 
        $dbpath = 'private/info_meteo.db';
        // Create (or open) the SQLite database
        $db = new SQLite3($dbpath);

        // Create the table if it doesn't exist
        $db->exec('CREATE TABLE IF NOT EXISTS meteo (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, temperature REAL, humidite REAL, luminosity REAL)');

        // Calculate the threshold timestamp for the last day
        $threshold = time() - $day_in_second; // 24 hours ago

        // Delete rows older than the last day
        $query = 'DELETE FROM meteo WHERE timestamp < :threshold';
        $stmt = $db->prepare($query);
        $stmt->bindValue(':threshold', $threshold, SQLITE3_INTEGER);
        $stmt->execute();

        $current_time = time();
        foreach ($data['data'] as $item){
            if (isset($item['temperature'], $item['luminosity'], $item['humidite'], $item['timestamp'])) {
                // Extract values
                $temperature = $item['temperature'];
                $luminosity = $item['luminosity'];
                $humidite = $item['humidite'];
                $timestamp = $item['timestamp'];

                error_log("Inserting Data: Timestamp: " . ($current_time + $timestamp) . ", Temperature: $temperature, Humidite: $humidite, luminosity: $luminosity");

                // Insert the new data into the table
                $stmt = $db->prepare('INSERT INTO meteo (timestamp, temperature, humidite, luminosity) VALUES (:timestamp, :temperature, :humidite, :luminosity)');
                $stmt->bindValue(':timestamp', $current_time + $timestamp, SQLITE3_INTEGER);
                $stmt->bindValue(':temperature', $temperature, SQLITE3_FLOAT);
                $stmt->bindValue(':humidite', $humidite, SQLITE3_FLOAT);
                $stmt->bindValue(':luminosity', $luminosity, SQLITE3_FLOAT);

                // Execute the statement
                $stmt->execute();
            }
        }
        // Close the database connection
        $db->close();

        // Success response
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
