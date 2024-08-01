$servername = "localhost"; 
$dBUsername = "dbname***"; //REPLACE with your Database name
$dBPassword = "username***"; //REPLACE with your Database user
$dBName = "Pass***"; //REPLACE with your Database password

$conn = mysqli_connect($servername, $dBUsername, $dBPassword, $dBName);
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}

/* Handle CORS */
header('Access-Control-Allow-Origin: *');
header("Access-Control-Allow-Headers: *");

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Mengambil data mentah dari permintaan
    $json = file_get_contents('php://input');

    // Mengonversinya menjadi objek PHP
    $data = json_decode($json);

    if (json_last_error() == JSON_ERROR_NONE) {
        $sampling = intval($data->Sampling);
        $MQ9 = floatval($data->MQ9);
        $RSSI = floatval($data->RSSI);
        
        $created = date('Y-m-d H:i:s');

        $response = '';
        $query = "INSERT INTO events (Sampling, MQ9, RSSI, created) VALUES ('$sampling', '$MQ9',  '$RSSI', '$created');";

        if (mysqli_query($conn, $query)) {
            $response = ['response' => 'Data baru berhasil dibuat :)'];
            header("HTTP/1.1 201 OK");
        } else {
            $response = ['response' => "Error: " . $query . "<br>" . mysqli_error($conn)];
            header("HTTP/1.1 500 NOT OK");
        }

        header('Content-Type: application/json; charset=utf-8');
        echo json_encode($response);

    } else {
        echo 'JSON tidak valid';
    }
}

// Handle all GET Requests
if ($_SERVER['REQUEST_METHOD'] === 'GET') {

    // Handle all GET Requests with query parameter "Sampling" for last 10 records
    if (isset($_GET['Sampling']) && isset($_GET['limit'])) {
        $sampling = $_GET['Sampling'];
        $limit = $_GET['limit'];

        $sql = "SELECT * FROM events WHERE Sampling = '$sampling' ORDER BY Sampling DESC LIMIT $limit;";
        $result = mysqli_query($conn, $sql);

        if ($result) {
            if ($limit > 1) {
                // Store the results in an array
                $response = array();
                while ($row = $result->fetch_assoc()) {
                    $response[] = $row;
                }
            } else {
                $sql = "SELECT * FROM events WHERE Sampling = '$sampling' ORDER BY Sampling DESC LIMIT 1;";
                $result = mysqli_query($conn, $sql);
                if (mysqli_num_rows($result) > 0) {
                    $row = mysqli_fetch_assoc($result);
                    $response = ['Sampling' => $row['Sampling'], 'MQ9' => $row['MQ9'], 'RSSI' => $row['RSSI'], 'created' => $row['created']];
                } else
                    $response = [];
            }

            header('Content-Type: application/json; charset=utf-8');
            echo json_encode($response);
        }
    }
}
// Handle PUT Requests
if ($_SERVER['REQUEST_METHOD'] === 'PUT') {
    // Takes raw data from the request
    $json = file_get_contents('php://input');

    // Converts it into a PHP object
    $data = json_decode($json);

    if (json_last_error() == JSON_ERROR_NONE) {
        $sampling = intval($data->Sampling);
        $MQ9 = floatval($data->MQ9);
        $RSSI = floatval($data->RSSI);
        

        // TODO: Handle the update in your database
        $query = "UPDATE events SET MQ9='$MQ9', RSSI='$RSSI' WHERE Sampling='$sampling';";

        if (mysqli_query($conn, $query)) {
            $response = ['response' => 'Data updated successfully :)'];
            header("HTTP/1.1 200 OK");
        } else {
            $response = ['response' => "Error: " . $query . "<br>" . mysqli_error($conn)];
            header("HTTP/1.1 500 Internal Server Error");
        }

        header('Content-Type: application/json; charset=utf-8');
        echo json_encode($response);

    } else {
        echo 'Invalid JSON';
    }
}
