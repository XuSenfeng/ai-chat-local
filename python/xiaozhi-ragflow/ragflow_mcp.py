import logging
import requests
from fastmcp import FastMCP
import sys
import time


# Fix UTF-8 encoding for Windows console
if sys.platform == 'win32':
    sys.stderr.reconfigure(encoding='utf-8')
    sys.stdout.reconfigure(encoding='utf-8')

# 配置日志
logger = logging.getLogger('ragflow_mcp')
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')


# Create an MCP server
mcp = FastMCP("ragflow_mcp")

@mcp.tool()
def get_search_results(question: str) -> dict:
    """
    Retrieve Ragflow search results based on the question.
    """
    logger.info(f"Retrieving results for question: {question}")
    
    # 直接调用API获取结果
    url = "http://localhost/api/v1/retrieval"
    headers = {
        "Content-Type": "application/json",
        "Authorization": "Bearer ragflow-NjODBiYWE4NmE4OTExZjA4ZWZkOWVhMz"
    }
    data = {
        "question": question,
        "dataset_ids": ["bc23625269e711f0b9229ea32eb1a8b3"],
        "document_ids": [],
        "highlight": False,
        "similarity_threshold": 0.30,
        "top_k": 64,
        "page_size": 2,
        "return_fields": ["content_ltks", "document_keyword", "similarity"]  # 只返回必要字段
    }
    
    try:
        # 发送请求
        start_time = time.time()
        response = requests.post(url, headers=headers, json=data, timeout=30)
        duration = time.time() - start_time
        logger.info(f"Search completed in {duration:.2f}s with status: {response.status_code}")
        
        # 处理响应
        if response.status_code == 200:
            result = []
            response_data = response.json()
            logger.info(f"Response data: {response_data}")
            for item in response_data.get("data", {}).get("chunks", []):
                # 处理每个结果项
                logger.info(f"Processing item: {item}")
                # 只提取必要字段
                result.append({
                    "content_ltks": item.get("content_ltks", ""),
                    "document_keyword": item.get("document_keyword", ""),
                    "similarity": item.get("similarity", 0.0),
                })
            
            # 返回结果
            return {
                "success": True,
                "result": result
            }
        else:
            error_msg = f"API error: {response.status_code} - {response.text[:200]}"
            return {"success": False, "error": error_msg}
    
    except requests.Timeout:
        error_msg = "Request timed out"
        logger.error(f"Search timed out")
        return {"success": False, "error": error_msg}
    except Exception as e:
        error_msg = str(e)
        logger.exception(f"Error processing search: {error_msg}")
        return {"success": False, "error": error_msg}


# Start the server
if __name__ == "__main__":
    try:
        logger.info("Starting Ragflow MCP server")
        mcp.run(transport="stdio")
    finally:
        logger.info("Ragflow MCP server shutdown complete")
