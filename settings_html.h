#ifndef SETTINGS_HTML_H
#define SETTINGS_HTML_H

const char* htmlContent = R"(

<html>
<head>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f4f4f4;
            color: #333;
        }

        h1 {
            text-align: center;
            color: #444;
        }

        p {
            margin: 15px;
        }

        input[type='range'] {
            width: 100%; /* Full-width sliders */
            margin: 10px 0;
        }

        @media (max-width: 600px) {
            h1 {
                font-size: 1.6em;
            }

            p {
                font-size: 0.9em;
            }

            input[type='range'] {
                width: 90%;
            }
        }
    </style>
    <script>
        function sendData() {
            // Get the values
            var flameHeight = document.getElementById('FlameHeight').value;
            var sparks = document.getElementById('Sparks').value;
            var delayDuration = document.getElementById('DelayDuration').value;

            // Update the display elements
            document.getElementById('FlameHeightValue').textContent = flameHeight;
            document.getElementById('SparksValue').textContent = sparks;
            document.getElementById('DelayDurationValue').textContent = delayDuration;

            // Send the data
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/update?FlameHeight=' + flameHeight
                + '&Sparks=' + sparks + '&DelayDuration=' + delayDuration, true);
            xhr.send();
        }

        // Function to initialize the display values on page load
        function initializeValues() {
            sendData(); // This will update the display elements with the initial values
        }

        document.addEventListener('DOMContentLoaded', function() {
            var FlameHeightSlider = document.getElementById('FlameHeight');
            var output = document.getElementById('FlameHeightValue');

            FlameHeightSlider.oninput = function() {
                output.innerHTML = this.value;
            }
        });

        document.addEventListener('DOMContentLoaded', function() {
            var SparksSlider = document.getElementById('Sparks');
            var output = document.getElementById('SparksValue');

            SparksSlider.oninput = function() {
                output.innerHTML = this.value;
            }
        });

        document.addEventListener('DOMContentLoaded', function() {
            var DelaySlider = document.getElementById('DelayDuration');
            var output = document.getElementById('DelayDurationValue');

            DelaySlider.oninput = function() {
                output.innerHTML = this.value;
            }
        });
    </script>
</head>
<body onload='initializeValues()'>
    <h1>Phoenix LED</h1>
    <p>FlameHeight: <input type='range' id='FlameHeight' min='0' max='100' onchange='sendData()'>
        <span id='FlameHeightValue'></span></p>
    <p>Sparks: <input type='range' id='Sparks' min='0' max='255' onchange='sendData()'>
        <span id='SparksValue'></span></p>
    <p>DelayDuration: <input type='range' id='DelayDuration' min='0' max='1000' onchange='sendData()'>
        <span id='DelayDurationValue'></span></p>
</body>
</html>
)";

#endif /* SETTINGS_HTML_H */
