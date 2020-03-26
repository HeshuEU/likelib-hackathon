pragma solidity >=0.4.0 <0.7.0;

contract AddressSend {
    uint256 coins_store;
    address payable minter;
    
    constructor() public {
        minter = msg.sender;
    }
    
    function testAddressSend(uint256 amount) public payable returns(bool is_success) {
        coins_store += msg.value;
        if(coins_store >= amount){
            bool result = minter.send(coins_store);
            if(result){
                coins_store = 0;
            }
	    return result;
        }
        return true;
    }

}
