// Function to switch between tabs
function showTab(tabName) {
    console.log("Switching to tab:", tabName);
    
    // Hide all tabs
    document.getElementById('options-tab').classList.add('hidden');
    document.getElementById('wifi-tab').classList.add('hidden');
    
    // Deactivate all tab headers
    document.getElementById('options-tab-btn').classList.remove('active');
    document.getElementById('wifi-tab-btn').classList.remove('active');
    
    // Show selected tab
    document.getElementById(tabName + '-tab').classList.remove('hidden');
    
    // Activate selected tab header
    document.getElementById(tabName + '-tab-btn').classList.add('active');
}

// Function to toggle WiFi password visibility
function togglePasswordVisibility() {
    const passwordField = document.getElementById('password');
    const passwordIcon = document.getElementById('password-icon');
    
    if (passwordField.type === 'password') {
        passwordField.type = 'text';
        passwordIcon.textContent = '👁️‍🗨️';  // Closed eye icon
    } else {
        passwordField.type = 'password';
        passwordIcon.textContent = '👁️';  // Open eye icon
    }
}

// Function to toggle Home Assistant token visibility
function toggleTokenVisibility() {
    const tokenField = document.getElementById('ha_token');
    const tokenIcon = document.getElementById('token-icon');
    
    if (tokenField.type === 'password') {
        tokenField.type = 'text';
        tokenIcon.textContent = '👁️‍🗨️';  // Closed eye icon
    } else {
        tokenField.type = 'password';
        tokenIcon.textContent = '👁️';  // Open eye icon
    }
}

// Function to scan for available WiFi networks
function scanNetworks() {
    const networksDiv = document.getElementById('networks');
    networksDiv.innerHTML = '<p>Scanning...</p>';
    
    fetch('/scan')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            networksDiv.innerHTML = '';
            
            if (!data.networks || data.networks.length === 0) {
                networksDiv.innerHTML = '<p>No networks found</p>';
                return;
            }
            
            // Sort networks by signal strength
            data.networks.sort((a, b) => b.rssi - a.rssi);
            
            data.networks.forEach(network => {
                const div = document.createElement('div');
                div.className = 'network';
                
                // Add signal strength indicator
                const signalStrength = Math.min(Math.max(Math.abs(network.rssi) - 30, 0) / 25, 4);
                const strengthClass = 'strength strength-' + Math.ceil(4 - signalStrength);
                
                div.innerHTML = network.ssid + 
                    '<span class="' + strengthClass + '">' + 
                    (network.secure ? '🔒' : '') + 
                    ' ' + network.rssi + 'dBm</span>';
                
                div.onclick = function() {
                    document.getElementById('ssid').value = network.ssid;
                    document.getElementById('password').focus();
                };
                
                networksDiv.appendChild(div);
            });
        })
        .catch(error => {
            console.error('Error:', error);
            networksDiv.innerHTML = '<p>Error scanning networks: ' + error.message + '</p>';
        });
}

// Initialize when DOM is fully loaded
document.addEventListener('DOMContentLoaded', function() {
    console.log("DOM fully loaded");
    
    // Make sure the form exists before adding event listener
    const configForm = document.getElementById('config-form');
    if (configForm) {
        configForm.addEventListener('submit', function(e) {
            const ssid = document.getElementById('ssid').value.trim();
            if (!ssid) {
                e.preventDefault();
                alert('Please enter a WiFi network name (SSID)');
                return false;
            }
            
            // Form is valid, allow submission
            return true;
        });
    } else {
        console.error("Config form not found!");
    }
    
    // Debug tab switching
    document.querySelectorAll('.tab').forEach(tab => {
        tab.addEventListener('click', function() {
            console.log('Tab clicked:', this.id);
        });
    });
});