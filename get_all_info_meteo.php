<?php
// Set the content type to JSON
header('Content-Type: application/json');

// Open the SQLite database
$dbpath = 'private/info_meteo.db';
$db = new SQLite3($dbpath);

// Query to get the latest meteorological data
$query = 'SELECT * FROM meteo ORDER BY id';
$result = $db->query($query);

$timestamps = [];
$temperatures = [];
$humidites = [];
$luminosity = [];


while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
    $timestamps[] = $row['timestamp'];     // Add to timestamps array
    $temperatures[] = $row['temperature']; // Add to temperatures array
    $humidites[] = $row['humidite'];       // Add to humidites array
    $luminosity[] = $row['luminosity'];       // Add to luminosities array
}


if (!empty($timestamps)) {
    echo json_encode([
        'timestamp' => $timestamps,
        'temperature' => $temperatures,
        'humidite' => $humidites,
        'luminosity' => $luminosity
    ]);
} else {
    // Return error if no data is found
    echo json_encode(['status' => 'error', 'message' => 'No data found']);
}

// Close the database connection
$db->close();
