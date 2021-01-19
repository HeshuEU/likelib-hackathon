pragma solidity >=0.4.0 <0.8.5;

contract GetPreviousBlock {

    function get() public view returns (bytes32 previous_block_hash) {
        return blockhash(block.number - 1);
    }

}
