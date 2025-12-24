#ifndef WEB_HTML_H
#define WEB_HTML_H

#include <Arduino.h>

const char WEB_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 RFID Manager</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: white;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.1);
            padding: 30px;
            border-radius: 15px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
            backdrop-filter: blur(10px);
        }
        h1 {
            text-align: center;
            margin-bottom: 30px;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.3);
        }
        .status-panel {
            background: rgba(255, 255, 255, 0.2);
            padding: 15px;
            border-radius: 10px;
            margin-bottom: 20px;
            text-align: center;
            font-size: 1.2em;
        }
        .button-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        button {
            padding: 15px 25px;
            font-size: 1.1em;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s ease;
            background: linear-gradient(45deg, #4CAF50, #45a049);
            color: white;
            font-weight: bold;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0, 0, 0, 0.3);
        }
        button:disabled {
            background: #cccccc;
            cursor: not-allowed;
            transform: none;
        }
        .read-btn { background: linear-gradient(45deg, #2196F3, #1976D2); }
        .write-btn { background: linear-gradient(45deg, #FF9800, #F57C00); }
        .format-btn { background: linear-gradient(45deg, #f44336, #d32f2f); }
        .card-data {
            background: rgba(255, 255, 255, 0.2);
            padding: 20px;
            border-radius: 10px;
            margin-top: 20px;
            display: none;
        }
        .write-form {
            background: rgba(255, 255, 255, 0.2);
            padding: 20px;
            border-radius: 10px;
            margin-top: 20px;
            display: none;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        input {
            width: 100%;
            padding: 10px;
            border: none;
            border-radius: 5px;
            font-size: 1em;
            box-sizing: border-box;
        }
        .device-info {
            text-align: center;
            margin-top: 20px;
            font-size: 0.9em;
            opacity: 0.8;
        }
        .spinner {
            display: none;
            width: 40px;
            height: 40px;
            border: 4px solid #f3f3f3;
            border-top: 4px solid #3498db;
            border-radius: 50%;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 RFID Manager</h1>
        
        <div class="status-panel" id="statusPanel">
            <div id="statusText">System ready</div>
            <div class="spinner" id="spinner"></div>
        </div>
        
        <div class="button-grid">
            <button class="read-btn" onclick="readCard()">Read Card</button>
            <button class="write-btn" onclick="showWriteForm()">Write Card</button>
            <button class="format-btn" onclick="formatCard()">Format Card</button>
        </div>
        
        <div class="card-data" id="cardData">
            <h3>Card Information</h3>
            <div id="cardInfo"></div>
        </div>
        
        <div class="write-form" id="writeForm">
            <h3>Write Card Data</h3>
            <form id="writeDataForm">
                <div class="form-group">
                    <label for="plate">License Plate:</label>
                    <input type="text" id="plate" name="plate" maxlength="16">
                </div>
                <div class="form-group">
                    <label for="vehicle">Vehicle Type:</label>
                    <input type="text" id="vehicle" name="vehicle" maxlength="16">
                </div>
                <div class="form-group">
                    <label for="department">Department:</label>
                    <input type="text" id="department" name="department" maxlength="16">
                </div>
                <button type="button" onclick="writeCard()">Write to Card</button>
                <button type="button" onclick="hideWriteForm()">Cancel</button>
            </form>
        </div>
        
        <div class="device-info">
            <p>Device Location: %LOCATION%</p>
            <p>Server IP: %SERVER_IP%</p>
        </div>
    </div>
    
    <script>
        function showSpinner() {
            document.getElementById('spinner').style.display = 'block';
        }
        
        function hideSpinner() {
            document.getElementById('spinner').style.display = 'none';
        }
        
        function updateStatus(message) {
            document.getElementById('statusText').textContent = message;
        }
        
        function readCard() {
            showSpinner();
            updateStatus('Reading card...');
            
            fetch('/read')
                .then(response => response.json())
                .then(data => {
                    hideSpinner();
                    if (data.success) {
                        updateStatus('Card read successfully!');
                        showCardData(data.data);
                    } else {
                        updateStatus(data.message || 'Failed to read card');
                    }
                })
                .catch(error => {
                    hideSpinner();
                    updateStatus('Error reading card');
                });
        }
        
        function showCardData(cardData) {
            const cardInfo = document.getElementById('cardInfo');
            cardInfo.innerHTML = '<p><strong>UID:</strong> ' + cardData.uid + '</p>' +
                '<p><strong>Type:</strong> ' + cardData.type + '</p>' +
                '<p><strong>Plate:</strong> ' + (cardData.plate || 'N/A') + '</p>' +
                '<p><strong>Vehicle:</strong> ' + (cardData.vehicle || 'N/A') + '</p>' +
                '<p><strong>Department:</strong> ' + (cardData.department || 'N/A') + '</p>';
            document.getElementById('cardData').style.display = 'block';
        }
        
        function showWriteForm() {
            document.getElementById('writeForm').style.display = 'block';
            document.getElementById('cardData').style.display = 'none';
        }
        
        function hideWriteForm() {
            document.getElementById('writeForm').style.display = 'none';
        }
        
        function writeCard() {
            const plate = document.getElementById('plate').value;
            const vehicle = document.getElementById('vehicle').value;
            const department = document.getElementById('department').value;
            
            if (!plate && !vehicle && !department) {
                updateStatus('Please enter at least one field');
                return;
            }
            
            showSpinner();
            updateStatus('Writing to card...');
            
            const formData = new URLSearchParams();
            formData.append('plate', plate);
            formData.append('vehicle', vehicle);
            formData.append('department', department);
            
            fetch('/writedata', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: formData
            })
            .then(response => response.json())
            .then(data => {
                hideSpinner();
                if (data.success) {
                    updateStatus('Data written successfully!');
                    hideWriteForm();
                    document.getElementById('writeDataForm').reset();
                } else {
                    updateStatus(data.message || 'Failed to write data');
                }
            })
            .catch(error => {
                hideSpinner();
                updateStatus('Error writing data');
            });
        }
        
        function formatCard() {
            if (!confirm('Are you sure you want to format this card? This will erase all data!')) {
                return;
            }
            
            showSpinner();
            updateStatus('Formatting card...');
            
            fetch('/format')
                .then(response => response.json())
                .then(data => {
                    hideSpinner();
                    if (data.success) {
                        updateStatus('Card formatted successfully!');
                        document.getElementById('cardData').style.display = 'none';
                    } else {
                        updateStatus(data.message || 'Failed to format card');
                    }
                })
                .catch(error => {
                    hideSpinner();
                    updateStatus('Error formatting card');
                });
        }
        
        setInterval(function() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    if (data.operation === 'idle') {
                        hideSpinner();
                    }
                })
                .catch(error => {
                    console.error('Status check error:', error);
                });
        }, 2000);
    </script>
</body>
</html>
)rawliteral";

#endif 