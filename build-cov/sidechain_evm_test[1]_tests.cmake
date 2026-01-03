add_test([=[EvmExecute.DisabledInFavorOfWasm]=]  /home/runner/work/BlockChainDrachma/BlockChainDrachma/build-cov/sidechain_evm_test [==[--gtest_filter=EvmExecute.DisabledInFavorOfWasm]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[EvmExecute.DisabledInFavorOfWasm]=]  PROPERTIES WORKING_DIRECTORY /home/runner/work/BlockChainDrachma/BlockChainDrachma/build-cov SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  sidechain_evm_test_TESTS EvmExecute.DisabledInFavorOfWasm)
