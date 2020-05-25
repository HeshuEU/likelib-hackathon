# JSON HTTP specification
##### Document with public http query specification of likelib node. API version: 1

---

### 1. get_node_info

request: 

	post to http:://<target url>/get_node_info

	### without body

response:

	### json object at body:
	{
		“method”: “get_node_info”,
		“status”: “ok”/”error”,
		“result”: {
			“top_block_hash”: “<block hash encoded by base64>”,
			“top_block_number”: <block number>,
			“api_version”: 1
		}
	}

### 2. get_account

request:

	post to http:://<target url>/get_account

	### need json object at body:
    {
        “address”: “<address encoded by base58>”,
    }

response:

	### json object at body:
	{
        “method”: “get_account”,
        “status”: “ok”/”error”,
        “result”: {
            “address”: “<address encoded by base58>”,
            “balance”: “<uint256 integer at string format>”,
            “nonce”: <integer>,
            "type": "client"/"contract"
            ## if "type" == "client" will be field "transaction_hashes"
            “transaction_hashes”: [<zero or more strings with hashes of transactions encoded by base64>]
		}
	}

### 3. get_block

request:

	post to http:://<target url>/get_block

	### need json object at body:
    {
        "hash": "<hash encoded by base64>"
        ## or
        “number”: <integer block number>,
    }

response:

	### json object at body:
	{
        “method”: “get_block”,
        “status”: “ok”/”error”,
        “result”: {
            “depth”: <integer>,
            “nonce”: <integer>,
            “timestamp”: <integer is seconds from epoch start>,
            “previous_block_hash”: “<block hash encoded by base64>”,
            “coinbase ”: “<address encoded by base58>”,
            “transactions”: [<one or more transactions objects(see push_transaction)>]
		}
	}

### 4. get_transaction

request:

	post to http:://<target url>/get_transaction

	### need json object at body:
    {
        "hash": "<hash encoded by base64>"
    }

response:

	### json object at body:
	{
		“method”: “get_transaction”,
		“status”: “ok”/”error”,
		“result”: <one transaction object(see push_transaction)>
	}

### 5. get_transaction_status

request:

	post to http:://<target url>/get_transaction_status

	### need json object at body:
    {
        "hash": "<hash encoded by base64>"
    }

response:

	### json object at body:
	{
		“method”: “get_transaction_status”,
		“status”: “ok”/”error”,
		“result”: {
			“status_code”: <number Success=0, Pending=1, BadQueryForm=2, BadSign=3, NotEnoughBalance=4, Revert=5, Failed=6>,
			“action_type”: <number None=0, Transfer=1, ContractCall=2, ContractCreation=3>,
			“fee_left”: “<uint256 integer at string format>”,
			“message”: “<All will be at such format if status_code == 0. If action_type == 1 then the message is empty string. If action_type == 2 then the message is encoded by base64 data from contract call in string type. If action_type == 3 then the message is address encoded by base58 in string type.>”,
		}
	}

### 6. push_transaction

request:

	post to http:://<target url>/push_transaction

	### need json object at body:
	{
		“from”: “<address encoded by base58>”,
		“to”: “<target address or null address if transaction for contract creation encoded by base58>”,
		“amount”: “<uint256 integer at string format>”,
		“fee”: “<uint256 integer at string format>”,
		“timestamp”: <integer is seconds from epoch start>,
		“data”: “<message ecnoded by base64 or empty string if "to" is not a contract address>”,
		“sign”: “<transaction hash signed by private key of sender(from address) encoded by base64 (see format notes for more information)>” 
	}

response:

	### json object at body:
	{
		“method”: “push_transaction”,
		“status”: “ok”/”error”,
		“result”: <transaction result object see get_transaction_status method>
	}

### 7. call_contract_view

request:

	post to http:://<target url>/call_contract_view

	### need json object at body:
	{
		“from”: “<address encoded by base58>”,
		“to”: “<contract address encoded by base58>”,
		“timestamp”: <integer is seconds from epoch start>,
		“message”: “<binary encoded(for call) data message ecnoded by base64>”,
		“sign”: “<call hash signed by private key of sender(from address) encoded by base64 (see format notes for more information)>”                                                               	
	}

response:

	### json object at body:
	{
		“method”: “call_contract_view”,
		“status”: “ok”/”error”,
		“result”: “<encoded by base64 data from contract call in string type>”,
	}

## Format notes:

- if “status” is “error” field “result” may be absent  or “result” will be a error message string.
- address is Ripemd160 of sha256 of serialized public key bytes.
- null address is 20 bytes of zeros.
- for sign using secp256k1. Hash of transaction using as signing message. singing function is sign_recoverable with sha256 hash function.
- transaction hash is sha256 of concatenated string:

		“<from address encoded by base58>” + “<to address or null address if transaction for contract creation encoded by base58>” + “<amount as uint256 integer at string format>” + “<fee as uint256 integer at string format>” + “<timestamp integer is seconds from epoch start at string>” + “<binary encoded data message ecnoded by base64 or empty string>”

- call hash is sha256 of concatenated string:
 
 		“<from address encoded by base58>” + “<to address encoded by base58>”  + “<timestamp integer is seconds from epoch start at string>” + “<binary encoded(for call) data message ecnoded by base64>”
     
   