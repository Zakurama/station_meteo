<?php
// Set the content type to JSON
header('Content-Type: application/json');

// Open the SQLite database
$dbpath = 'private/info_meteo.db';
$db = new SQLite3($dbpath);

// Query to get the latest meteorological data
$query = 'SELECT temperature, humidite, luminosity FROM meteo ORDER BY timestamp DESC LIMIT 1';
$result = $db->query($query);

// Check if data is found
if ($row = $result->fetchArray(SQLITE3_ASSOC)) {
    // Return data as JSON
    echo json_encode([
        'temperature' => $row['temperature'],
        'humidite' => $row['humidite'],
        'luminosity' => $row['luminosity']
    ]);
} else {
    // Return error if no data is found
    echo json_encode(['status' => 'error', 'message' => 'No data found']);
}

// Close the database connection
$db->close();
