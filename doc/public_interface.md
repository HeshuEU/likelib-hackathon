# Websocket public interface specification
##### Document with public websocket specification of likelib node.

Api version = 1.

##### query form to node

    {
		“type”: “subscribe”/"unsubscribe"/"call",
		"name": <command>,
		"api": 1,
		"id": <unsigned int which unique at current session and start from 1 and every query incrimented id>,
		“args”: {
			<args for command>
		}
	}

##### answer form from node

    {
		“type”: “answer",
		"id": <unsigned int equal to query id>,
		"status": "ok"/"error",
		“result”: <result object for command>
	}

##### call commands:

1. last_block_info

    query:

        {
            “type”: "call",
            "name": "last_block_info",
            "api": 1,
            "id": 99,
            “args”: {
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 99,
            "status": "ok",
            “result”: {
                “top_block_hash”: “<block hash encoded by base64>”,
                “top_block_number”: <block number>,
            }
        }

2. account_info

    query:

        {
            “type”: "call",
            "name": "account_info",
            "api": 1,
            "id": 56,
            “args”: {
                “address”: “<address encoded by base58>”,
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 56,
            "status": "ok",
            “result”: {
                “address”: “<address encoded by base58>”,
                “balance”: “<uint256 integer at string format>”,
                “nonce”: <integer>,
                "type": "client"/"contract"
                “transaction_hashes”: [<zero or more strings with hashes of transactions encoded by base64>]
            }
        }

3. find_block

    query:

        {
            “type”: "call",
            "name": "find_block",
            "api": 1,
            "id": 77,
            “args”: {
                "hash": "<hash encoded by base64>"
                ## or
                “number”: <integer block number>,
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 77,
            "status": "ok",
            “result”: {
                “depth”: <integer>,
                “nonce”: <integer>,
                “timestamp”: <integer is seconds from epoch start>,
                “previous_block_hash”: “<block hash encoded by base64>”,
                “coinbase ”: “<address encoded by base58>”,
                “transactions”: [<one or more transactions objects(see push_transaction)>]
            }
        }

4. find_transaction

    query:

        {
            “type”: "call",
            "name": "find_transaction",
            "api": 1,
            "id": 10,
            “args”: {
                "hash": "<hash encoded by base64>"
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 10,
            "status": "ok",
            “result”: {
            	“from”: “<address encoded by base58>”,
            	“to”: “<target address or null address if transaction for contract creation encoded by base58>”,
            	“amount”: “<uint256 integer at string format>”,
            	“fee”: “<uint256 integer at string format>”,
            	“timestamp”: <integer is seconds from epoch start>,
            	“data”: “<message ecnoded by base64 or empty string if "to" is not a contract address>”,
            	“sign”: “<transaction hash signed by private key of sender(from address) encoded by base64 (see format notes for more information)>” 
            }
        }

5. find_transaction_status

    query:

        {
            “type”: "call",
            "name": "find_transaction_status",
            "api": 1,
            "id": 21,
            “args”: {
                "hash": "<hash encoded by base64>"
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 21,
            "status": "ok",
            “result”: {
            	“status_code”: <number Success=0, Pending=1, BadQueryForm=2, BadSign=3, NotEnoughBalance=4, Revert=5, Failed=6>,
            	“action_type”: <number None=0, Transfer=1, ContractCall=2, ContractCreation=3>,
            	“fee_left”: “<uint256 integer at string format>”,
            	“message”: “<All will be at such format if status_code == 0. If action_type == 1 then the message is empty string. If action_type == 2 then the message is encoded by base64 data from contract call in string type. If action_type == 3 then the message is address encoded by base58 in string type.>”,
            }
        }

6. call_contract_view

    query:
    
        {
            “type”: "call",
            "name": "call_contract_view",
            "api": 1,
            "id": 67,
            “args”: {
            	“from”: “<address encoded by base58>”,
            	“to”: “<contract address encoded by base58>”,
            	“timestamp”: <integer is seconds from epoch start>,
            	“message”: “<binary encoded(for call) data message ecnoded by base64>”,
            	“sign”: “<call hash signed by private key of sender(from address) encoded by base64 (see format notes for more information)>”                                                               	
            }
        }

	answer:
	
        {
            "type": "answer",
            "id": 67,
            "status": "ok",
            "result": "<encoded by base64 data from contract call in string type>"
        }

##### subscribe commands:

1. last_block_info

    note: send answer when new block add

    query:

        {
            “type”: "subscribe",
            "name": "last_block_info",
            "api": 1,
            "id": 2,
            “args”: {
            }
        }

	answers:

        {
            “type”: "answer",
            "id": 2,
            "status": "ok",
            “result”: {
                “top_block_hash”: “<block hash encoded by base64>”,
                “top_block_number”: <block number>,
            }
        }

2. push_transaction

    note: send answer when transaction state change

    query:
    
        {
            “type”: "subscribe",
            "name": "push_transaction",
            "api": 1,
            "id": 53,
            “args”: {
            	“from”: “<address encoded by base58>”,
            	“to”: “<target address or null address if transaction for contract creation encoded by base58>”,
            	“amount”: “<uint256 integer at string format>”,
            	“fee”: “<uint256 integer at string format>”,
            	“timestamp”: <integer is seconds from epoch start>,
            	“data”: “<message ecnoded by base64 or empty string if "to" is not a contract address>”,
            	“sign”: “<transaction hash signed by private key of sender(from address) encoded by base64 (see format notes for more information)>” 
            }
        }

    answers:

        {
            “type”: "answer",
            "id": 53,
            "status": "ok",
            “result”: {
            	“status_code”: <number Success=0, Pending=1, BadQueryForm=2, BadSign=3, NotEnoughBalance=4, Revert=5, Failed=6>,
            	“action_type”: <number None=0, Transfer=1, ContractCall=2, ContractCreation=3>,
            	“fee_left”: “<uint256 integer at string format>”,
            	“message”: “<All will be at such format if status_code == 0. If action_type == 1 then the message is empty string. If action_type == 2 then the message is encoded by base64 data from contract call in string type. If action_type == 3 then the message is address encoded by base58 in string type.>”,
            }
        }

3. account_info

    note: send answer when account state change

    query:

        {
            “type”: "subscribe",
            "name": "account_info",
            "api": 1,
            "id": 80,
            “args”: {
                “address”: “<address encoded by base58>”,
            }
        }

	answers:

        {
            “type”: "answer",
            "id": 80,
            "status": "ok",
            “result”: {
                “address”: “<address encoded by base58>”,
                “balance”: “<uint256 integer at string format>”,
                “nonce”: <integer>,
                "type": "client"/"contract"
                “transaction_hashes”: [<zero or more strings with hashes of transactions encoded by base64>]
            }
        }

##### unsubscribe commands:

1. last_block_info

    note: send answer when new block add

    query:

        {
            “type”: "unsubscribe",
            "name": "last_block_info",
            "api": 1,
            "id": 9,
            “args”: {
            }
        }

	answers:

        {
            “type”: "answer",
            "id": 9,
            "status": "ok",
            “result”: "successful"/"error"
        }

2. push_transaction

    note: send answer when transaction state change

    query:
    
        {
            “type”: "unsubscribe",
            "name": "push_transaction",
            "api": 1,
            "id": 290,
            “args”: {
                "hash": "<hash encoded by base64>"
            }
        }

    answers:

        {
            “type”: "answer",
            "id": 290,
            "status": "ok",
            “result”: "successful"/"error"
        }

3. account_info

    note: send answer when account state change

    query:

        {
            “type”: "unsubscribe",
            "name": "account_info",
            "api": 1,
            "id": 122,
            “args”: {
                “address”: “<address encoded by base58>”,
            }
        }

	answers:

        {
            “type”: "answer",
            "id": 122,
            "status": "ok",
            “result”: "successful"/"error"
        }

## Format notes:

- Address is Ripemd160 of sha256 of serialized public key bytes.
- Null address is 20 bytes of zeros.
- for sign using secp256k1. Hash of transaction using as signing message. Singing function is sign_recoverable with sha256 hash function.
- transaction hash is sha256 of concatenated string:

		“<from address encoded by base58>” + “<to address or null address if transaction for contract creation encoded by base58>” + “<amount as uint256 integer at string format>” + “<fee as uint256 integer at string format>” + “<timestamp integer is seconds from epoch start at string>” + “<binary encoded data message ecnoded by base64 or empty string>”

- call hash is sha256 of concatenated string:
 
 		“<from address encoded by base58>” + “<to address encoded by base58>”  + “<timestamp integer is seconds from epoch start at string>” + “<binary encoded(for call) data message ecnoded by base64>”
     
   