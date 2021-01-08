pragma solidity >=0.4.0 <0.8.5;

contract SimpleStorage {
    uint storedData;

    constructor(uint x) public{
        storedData = x;
    }

    function set(uint x) public {
        storedData = x;
    }

    function get() public view returns (uint stored_data) {
        return storedData;
    }
}
