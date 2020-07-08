from tester import test_case, Node, NodePool
#from tester import test_case, Node, NodePool
import time
import random

""" Второй пул видит только ноды из первого пула.
 После запуска двух пулов даётся время на синхронизацию (ожидание),
 Во втором пуле ноды делают по одной транзакции каждая, ожидание
 Все ноды первого и второго пула проверяют все транзакции
 Если ноды во втором пуле не видят друг друга, тест свалится
 Если ноды первого пула всё проверили, а во втором ошибки - значит ноды первого пула не передали блоки, считая что во втором пуле ноды уже видят друг друга
 Иногда тест падает, если transf_timeout слишком маленький (увеличить его или уменьшить сложность майнинга) """

@test_case("real_network_2_pools_non_stop")
def main(env, logger):

  pool_cfg_1 = {'name':"first" , 'nodes':1, 'timeout':1, 'mining_thr':1,
                'start_sync_port':20100, 'start_rpc_port':50100}
  pool_cfg_2 = {'name':"second", 'nodes':2, 'timeout':1, 'mining_thr':1,
                'start_sync_port':20200, 'start_rpc_port':50200}
  sync_timeout   = 2
  transf_timeout = 4
  transf_sum = 100
  with NodePool.create_every_to_previous(env, logger, pool_cfg_1['start_sync_port'],
                pool_cfg_1['start_rpc_port'], pool_cfg_1['nodes'], pool_cfg_1['mining_thr']) as nodes_1:
    nodes_1.start_nodes(pool_cfg_1['timeout'])
    for node in nodes_1:
      node.run_check_test()
    with NodePool.create_every_to_custom_list(env,logger, pool_cfg_2['start_sync_port'],
                pool_cfg_2['start_rpc_port'], pool_cfg_2['nodes'], nodes_1.ids,
                pool_cfg_2['mining_thr']) as nodes_2:

      nodes_2.start_nodes(pool_cfg_2['timeout'])
      for node in nodes_2:
        node.run_check_test()
      time.sleep(sync_timeout)
      target_addresses = []
      for node in nodes_2:
        target_addresses.append(node.create_new_address(keys_path="key1"))
        node.run_check_balance(target_addresses[-1], 0)
        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        node.run_check_transfer(to_address=target_addresses[-1], amount=transf_sum,
                                from_address=distributor_address, fee=0,
                                timeout=transf_timeout, wait=0)
        node.run_check_balance(target_addresses[-1], transf_sum)
      time.sleep(sync_timeout)
      for node in nodes_1:
        for target_addr in target_addresses:
          node.run_check_balance(target_addr, transf_sum)
      for node in nodes_2:
        for target_addr in target_addresses:
          node.run_check_balance(target_addr, transf_sum)

  return 0


""" Второй пул видит только ноды из первого пула.
 После запуска двух пулов даётся время на синхронизацию (ожидание), первый пул отключается, ожидание
 Во втором пуле ноды делают по одной транзакции каждая, ожидание
 Все ноды второго пула проверяют все транзакции
 Если ноды во втором пуле не видят друг друга, тест свалится
 Иногда тест падает, если transf_timeout слишком маленький (увеличить его или уменьшить сложность майнинга) """

@test_case("real_network_2_pools_first_pool_stopped")
def main(env, logger):

  pool_cfg_1 = {'name':"first" , 'nodes':1, 'timeout':1, 'mining_thr':1,
                'start_sync_port':20100, 'start_rpc_port':50100}
  pool_cfg_2 = {'name':"second", 'nodes':2, 'timeout':1, 'mining_thr':1,
                'start_sync_port':20200, 'start_rpc_port':50200}
  sync_timeout   = 2
  transf_timeout = 4
  transf_sum = 100
  with NodePool.create_every_to_previous(env, logger, pool_cfg_1['start_sync_port'],
                pool_cfg_1['start_rpc_port'], pool_cfg_1['nodes'], pool_cfg_1['mining_thr']) as nodes_1:
    nodes_1.start_nodes(pool_cfg_1['timeout'])
    for node in nodes_1:
      node.run_check_test()
    with NodePool.create_every_to_custom_list(env,logger, pool_cfg_2['start_sync_port'],
                pool_cfg_2['start_rpc_port'], pool_cfg_2['nodes'], nodes_1.ids,
                pool_cfg_2['mining_thr']) as nodes_2:

      nodes_2.start_nodes(pool_cfg_2['timeout'])
      for node in nodes_2:
        node.run_check_test()
      time.sleep(sync_timeout)
      for node in nodes_1: node.close()
      time.sleep(sync_timeout)
      target_addresses = []
      for node in nodes_2:
        target_addresses.append(node.create_new_address(keys_path="key1"))
        node.run_check_balance(target_addresses[-1], 0)
        distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
        node.run_check_transfer(to_address=target_addresses[-1], amount=transf_sum,
                                from_address=distributor_address, fee=0,
                                timeout=transf_timeout, wait=0)
        node.run_check_balance(target_addresses[-1], transf_sum)
      time.sleep(sync_timeout)
      for node in nodes_2:
        for target_addr in target_addresses:
          node.run_check_balance(target_addr, transf_sum)

  return 0




""" Идея для теста - запустить три пула нод:
 - Первый достоверный, они всегда работают и грантированно отвечают (сервера)
 - Второй переодически отключает поочерёдно ноды,
      а потом включает их с сохранением базы (типа юзеры)
 - Третий имеет проблемы с сетью (эмулируются задержки и потери пакетов)
        или
 - Третий - это клисенты которые приходят и уходят, очищая базу
 Проводятся транзакции во втором и третьем пуле
 После некоторого времени работы, проверяется синхронность нод """

# На данный момент реализован первый и второй пул - как задумано
# Третий пул - просто проводит транзакции (каждая нода)
# Все три пула производят проверку произведённых транзакций
# Если какой то транзакции нет - тест падает


@test_case("real_network_3_pools")
def main(env, logger):

  pool_cfg_1 = {'name':"first" , 'nodes':1, 'timeout':1, 'mining_thr':1,
                'start_sync_port':20100, 'start_rpc_port':50100, 'clean_db':False}
  pool_cfg_2 = {'name':"second", 'nodes':5, 'timeout':1, 'mining_thr':1,
                'start_sync_port':20200, 'start_rpc_port':50200, 'clean_db':False}
  pool_cfg_3 = {'name':"second", 'nodes':2, 'timeout':1, 'mining_thr':1,
                'start_sync_port':20300, 'start_rpc_port':50300, 'clean_db':True}
  sync_timeout   = 2
  transf_timeout = 4
  transf_sum = 100
  with NodePool.create_every_to_previous(env, logger, pool_cfg_1['start_sync_port'],
                pool_cfg_1['start_rpc_port'], pool_cfg_1['nodes'], pool_cfg_1['mining_thr']) as nodes_1:
    nodes_1.start_nodes(pool_cfg_1['timeout'])
    for node in nodes_1:
      node.run_check_test()
    with NodePool.create_every_to_custom_list(env,logger, pool_cfg_2['start_sync_port'],
                pool_cfg_2['start_rpc_port'], pool_cfg_2['nodes'], nodes_1.ids,
                pool_cfg_2['mining_thr']) as nodes_2:

      nodes_2.start_nodes(pool_cfg_2['timeout'])
      for node in nodes_2:
        node.run_check_test()
      with NodePool.create_every_to_custom_list(env,logger, pool_cfg_3['start_sync_port'],
                  pool_cfg_3['start_rpc_port'], pool_cfg_3['nodes'], nodes_1.ids,
                  pool_cfg_3['mining_thr']) as nodes_3:

        nodes_3.start_nodes(pool_cfg_3['timeout'])
        for node in nodes_3:
          node.run_check_test()
        time.sleep(sync_timeout)
        # Все три пула запущены и синхронизированны
        down_nodes = []
        target_addresses = []
        # Закрываем половину нод из 2 пула (меньшую половину, если нечётное)
        for i in range(0, pool_cfg_2['nodes']//2):
          node = nodes_2.pop(random.randrange(len(nodes_2)))
          node.close()
          down_nodes.append(node)
          logger.debug("Node: " + str(node.settings.id.listen_rpc_address) + " down")
        # В третьем пуле каждая нода делает по одной транзакции
        for node in nodes_3:
          target_addresses.append(node.create_new_address(keys_path="key1"))
          node.run_check_balance(target_addresses[-1], 0)
          distributor_address = node.load_address(keys_path=Node.DISTRIBUTOR_ADDRESS_PATH)
          node.run_check_transfer(to_address=target_addresses[-1], amount=transf_sum,
                                from_address=distributor_address, fee=0,
                                timeout=transf_timeout, wait=0)
          node.run_check_balance(target_addresses[-1], transf_sum)
        time.sleep(sync_timeout)
        # Запуск всех остановленых нод
        for i in range(0, len(down_nodes)):
          node = down_nodes.pop()
          node.start_node(pool_cfg_2['timeout'])
          nodes_2.append(node)
          logger.debug("Node: " + str(node.settings.id.listen_rpc_address) + " started")
        for node in nodes_2:
          node.run_check_test()
        logger.debug("All nodes in nodes_2 started. Synchronyzation")
        time.sleep(sync_timeout)
        # Проверка синхронности
        for node in nodes_1:
          for target_addr in target_addresses:
            node.run_check_balance(target_addr, transf_sum)
        for node in nodes_3:
          for target_addr in target_addresses:
            node.run_check_balance(target_addr, transf_sum)
        for node in nodes_2:
          for target_addr in target_addresses:
            node.run_check_balance(target_addr, transf_sum)


  return 0
