pragma solidity >=0.4.0 <0.7.0;

contract CodeHash {
    function getCodeHash(address addr) public view returns (uint256 hash){
  	    assembly { hash := extcodehash(addr) }
    }

}
