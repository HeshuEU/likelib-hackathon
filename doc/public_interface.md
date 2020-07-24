# WebSocket public interface specification
##### Document with public websocket specification of Likelib node.

API version = 2.

---

### Common query/answer templates
###### query form to node

    {
        “type”: “subscribe”/"unsubscribe"/"call",
        "name": <command name>,
        "version": 2,
        "id": <unsigned integer which unique at current WS session>,
        “args”: {
            <args for command>
        }
    }

###### good answer from node

    {
        “type”: “answer",
        "id": <unsigned integer equal to query id>,
        "status": "ok",
        “result”: <result object for command>
    }

###### answer if smt goes wrong

    {
        “type”: “answer",
        "id": <unsigned int equal to query id>,
        "status": "error",
        “result”: {
            "error_message": "<error message in plane text>"
        }
    }

---

### Details notes

- Address is Ripemd160 of SHA256 of serialized public key bytes.
- Null address is 20 bytes of zeros.
- for sign using secp256k1. Hash of transaction using as signing message. Singing function is sign_recoverable with SHA256 function.
- transaction hash is SHA256 of concatenated string:

		“<from address encoded by base58>” + “<to address or null address if transaction for contract creation encoded by base58>” + “<amount as uint256 integer at string format>” + “<fee as uint256 integer at string format>” + “<timestamp integer is seconds from epoch start at string>” + “<binary encoded data message ecnoded by base64 or empty string>”

- call hash is sha256 of concatenated string:
 
 		“<from address encoded by base58>” + “<to address encoded by base58>”  + “<timestamp integer is seconds from epoch start at string>” + “<binary encoded(for call) data message ecnoded by base64>”

---

### Commands query/answer specification

##### 1. Get(once) top(last in chain) block information(hash and number)

    query:

        {
            “type”: "call",
            "name": "top_block_info",
            "version": 2,
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

##### 2. Subscribe on top(last in chain) block information(hash and number) updating

    query:

        {
            “type”: "subscribe",
            "name": "top_block_info",
            "version": 2,
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

##### 3. Cancel subscription on top(last in chain) block information(hash and number) updating

    query:

        {
            “type”: "unsubscribe",
            "name": "top_block_info",
            "version": 2,
            "id": 9,
            “args”: {
            }
        }

	answers:

        {
            “type”: "answer",
            "id": 9,
            "status": "ok",
            “result”: {
                "success": true,
            }
        }

        ## if subscription is not exist 

        {
            “type”: "answer",
            "id": 9,
            "status": "ok",
            “result”: {
                "success": false,
            }
        }
 // TODO SET MORE INFO(ABOUT) AT ACCOUNT STATE INFO
##### 4. Get(once) current account state info 

    query:

        {
            “type”: "call",
            "name": "account_state",
            "version": 2,
            "id": 56,
            “args”: {
                “address”: “<address encoded by base58>”,
            }
        }

	answer:
	
    if address of client account:

        {
            “type”: "answer",
            "id": 56,
            "status": "ok",
            “result”: {
                "exist": true,
                “address”: “<address encoded by base58>”,
                "type": "client",
                “balance”: “<uint256 at string format>”,
                “nonce”: <unsigned integer.>,
                “transaction_hashes”: [<zero or more strings with hashes of transactions encoded by base64>]
            }
        }

    if address of contract account:

        {
            “type”: "answer",
            "id": 56,
            "status": "ok",
            “result”: {
                "exist": true,
                “address”: “<address encoded by base58>”,
                "type": "contract",
                “balance”: “<uint256 at string format>”,
                “runtime_bytecode”: <contract runtime bytecode encoded by base64>,
                “memory_state”: {
                    "<32 bytes address encoded by base64>": "<32 bytes word value encoded by base64>"
                    ....
                }
            }
        }

    if contract account is not exist or client account didn't take part in any transaction:

        {
            “type”: "answer",
            "id": 56,
            "status": "ok",
            “result”: {
                "exist": false,
                “address”: “<address encoded by base58>”
            }
        }

##### 5. Subscribe on account's state updates

    query:

        {
            “type”: "subscribe",
            "name": "account_state",
            "version": 2,
            "id": 80,
            “args”: {
                “address”: “<address encoded by base58>”,
            }
        }

	answers:
	
    if address of client account:

        {
            “type”: "answer",
            "id": 80,
            "status": "ok",
            “result”: {
                “address”: “<address encoded by base58>”,
                "type": "client",
                “balance”: “<uint256 at string format>”,
                “nonce”: <unsigned integer.>,
                “transaction_hashes”: [<zero or more strings with hashes of transactions encoded by base64>]
            }
        }

    if address of contract account:

        {
            “type”: "answer",
            "id": 80,
            "status": "ok",
            “result”: {
                “address”: “<address encoded by base58>”,
                "type": "contract",
                “balance”: “<uint256 at string format>”,
                “runtime_bytecode”: <contract runtime bytecode encoded by base64>,
                “memory_state”: {
                    "<32 bytes address encoded by base64>": "<32 bytes word value encoded by base64>"
                    ....
                }
            }
        }


##### 6. Cancel subscription on account's state updates

    query:

        {
            “type”: "unsubscribe",
            "name": "account_state",
            "version": 2,
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
            “result”: {
                "success": true,
            }
        }

        ## if subscription is not exist 

        {
            “type”: “answer",
            "id": 122,
            "status": "ok",
            “result”: {
                "success": false,
            }
        }

##### 7. Push a transaction and subscribe on the transaction's status updates

    query:

        {
            “type”: "subscribe",
            "name": "push_transaction",
            "version": 2,
            "id": 53,
            “args”: {
                "hash": "<hash encoded by base64>",
                “from”: “<address encoded by base58>”,
                “to”: “<target address(or null address if transaction for contract creation) encoded by base58>”,
                “amount”: “<uint256 at string format>”,
                “fee”: “<uint256 at string format>”,
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
                "hash": "<hash of transaction encoded by base64>",
                “status_code”: <number Success=0, Pending=1, BadQueryForm=2, BadSign=3, NotEnoughBalance=4, Revert=5, Failed=6>,
                “action_type”: <number None=0, Transfer=1, ContractCall=2, ContractCreation=3>,
                “fee_left”: “<uint256 integer at string format>”,
                “message”: “<All will be at such format if status_code == 0. If action_type == 1 then the message is empty string. If action_type == 2 then the message is encoded by base64 data from contract call in string type. If action_type == 3 then the message is address encoded by base58 in string type.>”,
            }
        }

##### 8. Get(once) transaction data(transaction object) if exist

    query:

        {
            “type”: "call",
            "name": "transaction",
            "version": 2,
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
               "exist": true,
               "hash": "<hash encoded by base64>",
               “from”: “<address encoded by base58>”,
               “to”: “<target address(or null address if transaction for contract creation) encoded by base58>”,
               “amount”: “<uint256 at string format>”,
               “fee”: “<uint256 at string format>”,
               “timestamp”: <integer is seconds from epoch start>,
               “data”: “<message ecnoded by base64 or empty string if "to" is not a contract address>”,
               “sign”: “<transaction hash signed by private key of sender(from address) encoded by base64 (see format notes for more information)>” 
            }
        }

    if transaction is not exisits:

        {
            “type”: "answer",
            "id": 10,
            "status": "ok",
            “result”: {
                "exist": false,
                "hash": "<hash encoded by base64>"
            }
        }

##### 9. Get(once) current transaction's status if transaction exist

    query:

        {
            “type”: "call",
            "name": "transaction_status",
            "version": 2,
            "id": 21,
            “args”: {
                "hash": "<hash of transaction encoded by base64>"
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 21,
            "status": "ok",
            “result”: {
                "exist": true,
                "hash": "<hash of transaction encoded by base64>",
                “status_code”: <number Success=0, Pending=1, BadQueryForm=2, BadSign=3, NotEnoughBalance=4, Revert=5, Failed=6>,
                “action_type”: <number None=0, Transfer=1, ContractCall=2, ContractCreation=3>,
                “fee_left”: “<uint256 integer at string format>”,
                “message”: “<All will be at such format if status_code == 0. If action_type == 1 then the message is empty string. If action_type == 2 then the message is encoded by base64 data from contract call in string type. If action_type == 3 then the message is address encoded by base58 in string type.>”,
            }
        }

    if transaction is not exisits:

        {
            “type”: "answer",
            "id": 21,
            "status": "ok",
            “result”: {
                "exist": false,
                "hash": "<hash of transaction encoded by base64>"
            }
        }

##### 10. Subscribe on transaction's status updates

    query:

        {
            “type”: "subscribe",
            "name": "transaction_status",
            "version": 2,
            "id": 50,
            “args”: {
                "hash": "<hash encoded by base64>"
            }
        }

    answers:

        {
            “type”: "answer",
            "id": 50,
            "status": "ok",
            “result”: {
                "hash": "<hash of transaction encoded by base64>",
                “status_code”: <number Success=0, Pending=1, BadQueryForm=2, BadSign=3, NotEnoughBalance=4, Revert=5, Failed=6>,
                “action_type”: <number None=0, Transfer=1, ContractCall=2, ContractCreation=3>,
                “fee_left”: “<uint256 integer at string format>”,
                “message”: “<All will be at such format if status_code == 0. If action_type == 1 then the message is empty string. If action_type == 2 then the message is encoded by base64 data from contract call in string type. If action_type == 3 then the message is address encoded by base58 in string type.>”,
            }
        }

##### 11. Cancel subscription on transaction's status updates 

    query:

        {
            “type”: "unsubscribe",
            "name": "transaction_status",
            "version": 2,
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
            “result”: {
                "success": true,
            }
        }

        ## if subscription is not exist 

        {
            “type”: "answer",
            "id": 290,
            "status": "ok",
            “result”: {
                "success": false,
            }
        }

##### 12. Get(once) light block data if exist

    query:

        {
            “type”: "call",
            "name": "light_block",
            "version": 2,
            "id": 79,
            “args”: {
                "hash": "<hash encoded by base64>"
                ## or
                “depth”: <unsigned integer block number>,
                ## if had two options using, node would use "hash" option. 
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 79,
            "status": "ok",
            “result”: {
                "exist": true,
                "hash": "<hash encoded by base64>",
                “depth”: <unsigned integer block number>,
                “nonce”: <unsigned integer magic>,
                “timestamp”: <unsigned integer is seconds from epoch start>,
                “previous_block_hash”: “<block hash encoded by base64>”,
                “coinbase ”: “<address encoded by base58>”,
                “transaction_hashes”: [<one or more strings with hashes of transactions encoded by base64>]
            }
        }

    if block is not exisits:

        {
            “type”: "answer",
            "id": 79,
            "status": "ok",
            “result”: {
                "exist": false,
                "hash": "<hash encoded by base64>"
                ## if exist only depth
                “depth”: <unsigned integer block number>
            }
        }

##### 13. Get(once) full block data if exist

    query:

        {
            “type”: "call",
            "name": "full_block",
            "version": 2,
            "id": 77,
            “args”: {
                "hash": "<hash encoded by base64>"
                ## or
                “depth”: <unsigned integer block number>,
                ## if two options are exist, node will use "hash" option. 
            }
        }

	answer:

        {
            “type”: "answer",
            "id": 77,
            "status": "ok",
            “result”: {
                "exist": true,
                "hash": "<hash encoded by base64>",
                “depth”: <unsigned integer block number>,
                “nonce”: <unsigned integer magic>,
                “timestamp”: <unsigned integer is seconds from epoch start>,
                “previous_block_hash”: “<block hash encoded by base64>”,
                “coinbase ”: “<address encoded by base58>”,
                “transactions”: [<one or more transactions objects>]
            }
        }

    if block is not exisits:

        {
            “type”: "answer",
            "id": 77,
            "status": "ok",
            “result”: {
                "exist": false,
                "hash": "<hash encoded by base64>"
                ## if exist only depth
                “depth”: <unsigned integer block number>
            }
        }
