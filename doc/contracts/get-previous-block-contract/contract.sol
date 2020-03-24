pragma solidity >=0.4.0 <0.7.0;

contract GetPreviousBlock {

    function get() public view returns (bytes32 previous_block_hash) {
        return blockhash(block.number - 1);
    }

}
