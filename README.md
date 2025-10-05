### Port Scanner

Usage Examples:

🔹 Scan all 65535 ports:
<pre>
./port_scanner --ip 192.168.1.1 --all
Scanning all 65535 ports...

Port 22 is OPEN
Port 445 is OPEN
Port 5000 is OPEN
Port 5432 is OPEN
Port 7000 is OPEN
</pre>

🔹 Scan a specific port (e.g., 22):
<pre>
MacBook-Air-2:port-scanner anish$ ./port_scanner --ip 192.168.29.79 --port 22
Port 22 is OPEN
</pre>

🔹 Scan multiple ports (e.g., 22, 80, 443)
<pre>
MacBook-Air-2:port-scanner anish$ ./port_scanner --ip 192.168.29.79 --ports 22,80,443
Port 22 is OPEN
</pre>

🔹 Scan top 100 ports
<pre>
MacBook-Air-2:port-scanner anish$ ./port_scanner --ip 192.168.29.79 --top100
Scanning top 100 common ports...

Port 22 is OPEN
Port 445 is OPEN
Port 5000 is OPEN
Port 5432 is OPEN
</pre>

🔹 Scan a range of ports (e.g., 20-100):
<pre>
MacBook-Air-2:port-scanner anish$ ./port_scanner --ip 192.168.29.79 --range 20-100
Scanning port range 20-100...

Port 22 is OPEN
</pre>

🔹 Adjust the number of threads (e.g., 100 threads for faster scanning):
<pre>
MacBook-Air-2:port-scanner anish$ ./port_scanner --ip 192.168.29.79 --all --threads 100
Scanning all 65535 ports...

Port 22 is OPEN
Port 445 is OPEN
Port 5000 is OPEN
Port 5432 is OPEN
Port 7000 is OPEN
</pre>
