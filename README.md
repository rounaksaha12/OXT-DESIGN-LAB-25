# OXT-DESIGN-LAB-25

For system requirements and dependencies, follow [TWINSSE](https://github.com/SEAL-IIT-KGP/TWINSSE/tree/main?tab=readme-ov-file) github repo. 

## Running the experiments

The first objective of the project is to separate the server and client codes from [TWINSSE](https://github.com/SEAL-IIT-KGP/TWINSSE/tree/main?tab=readme-ov-file) to perform SSE setup and search using Oblivious Cross Tag (OXT) protocol. The server code would run inside a CentOS virtual machine hosted via QEMU, while the client code would run on the local machine. Socket-based communication between the client and server is implemented in C++.

`OXT_CONJ_SERVER` is the server side code and should be placed in the virtual machine while `OXT_CONJ_CLIENT` is the client side code and should be placed in local machine.

### Launching qemu

Since the server code binds to port 8080 inside the CentOS virtual machine, QEMU is launched with port forwarding enabled, mapping a port on the host to port 8080 in the guest, to allow the client running locally to communicate with the server.

```
qemu-system-x86_64 \
  ... \
  -net nic \
  -net user,hostfwd=tcp::8080-:8080 \
  ...
```

In case of connection issues, disabling the firewall on CentOS may be necessary.

```
sudo systemctl stop firewalld
```

Navigate to `OXT_CONJ_CLIENT/client` in client(local machine) and to `OXT_CONJ_SERVER/server` in server(virtual machine).

### SSE Setup

Follow the makefiles to build the targets `sse_setup_client` and `sse_setup_server` in the client and server respectively.

Run in server,

```
./sse_setup_server
```
and then in client,
```
./sse_setup_client
```

Client will read the database and send the server: 
* TSet entries (which are written to the redis database in the server)
* XSet bloomfilter (written to the disk as `bloomfilter.dat`)

### SSE Search

Follow the makefiles to build the targets `sse_search_client` and `sse_search_server` in the client and server respectively.

(Optionally) Run `OXT_CONJ_CLIENT/databases/create_testcase.py` that creates two files: `input.txt` and `exp_output.txt`. Each line of `input.txt` is a pair of keywords $w_1$, $w_2$ and the corresponding line is `exp_output.txt` contains the list of docids that contain $w_1 \wedge w_2$. It is guaranteed that the intersection of docids for any keyword pair generated would be non-empty.

Note that `OXT_CONJ_CLIENT/client/results` currently has `input.txt` and `exp_output.txt` with 4 conjunctive queries.

Also note that the variable `n_iterations` in `OXT_CONJ_CLIENT/client/sse_search_client.cpp` should be initialized with a value greater than the number of lines in `input.txt`.

Finally, 
Run in server,

```
./sse_search_server
```
and then in client,
```
./sse_search_client
```

Note that the **client program waits for the user to press Enter after each query is performed to read the next line from** `input.txt`

After all queries are done, run `OXT_CONJ_CLIENT/client/results/evaluate_correctness.py` to verify if the server returned correct docids.