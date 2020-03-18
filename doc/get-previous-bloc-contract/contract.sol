pragma solidity >=0.4.0 <0.7.0;

contract GetPreviousBlock {
    uint public blockNumber;
    bytes32 public blockHashNow;

    function setValues() public {
        blockNumber = block.number;
        blockHashNow = blockhash(blockNumber - 1);
    }

    function get() public view returns (bytes32) {
        return blockHashNow;
    }
}