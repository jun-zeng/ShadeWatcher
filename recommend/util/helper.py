import os

from util.setting import logger

def ensureDir(dir_path: str) -> None:
    """Check whether directory exists or not.
    
    If directory doesnot existed, then create one.
    """
    dir = os.path.dirname(dir_path)
    if not os.path.exists(dir):
        os.makedirs(dir)

def printCooMatList(mat_list: list) -> None:
    """Printing content of CooMatList for debugging.
    """
    logger.debug('[\n')
    
    for coo in mat_list:
        logger.debug(coo.todense())
    
    logger.debug(']\n')
