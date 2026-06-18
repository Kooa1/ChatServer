"""
ChatServer 压测工具
测量指标：连接速率、请求延迟、吞吐量、并发容量
用法: python benchmark.py [--host HOST] [--port PORT]
输出: benchmark_result.md（测试结果报告）
"""

import argparse
import json
import os
import socket
import struct
import sys
import time
import threading
import statistics
from datetime import datetime
from concurrent.futures import ThreadPoolExecutor, as_completed

MAGIC = 0x4A3B2C1D
CMD_LOGIN = 0x001

# 全局结果收集
_output_lines = []


def log(msg=""):
    _output_lines.append(msg)
    print(msg)


def log_separator(title=None):
    if title:
        log()
        log("=" * 60)
        log(f"  {title}")
        log("=" * 60)
    else:
        log("-" * 60)


def build_packet(cmd_type: int, payload: dict) -> bytes:
    body = json.dumps(payload, separators=(",", ":")).encode("utf-8")
    ts = int(time.time() * 1000)
    header = struct.pack("!IIQI", MAGIC, cmd_type, ts, len(body))
    return header + body


def recv_response(sock: socket.socket, timeout=5.0) -> bytes:
    sock.settimeout(timeout)
    header = b""
    while len(header) < 20:
        chunk = sock.recv(20 - len(header))
        if not chunk:
            raise ConnectionError("connection closed")
        header += chunk
    _, _, _, body_len = struct.unpack("!IIQI", header)
    body = b""
    while len(body) < body_len:
        chunk = sock.recv(body_len - len(body))
        if not chunk:
            raise ConnectionError("connection closed during body read")
        body += chunk
    return body


def check_server(host, port):
    """检查服务器是否可达"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect((host, port))
        sock.close()
        return True
    except Exception as e:
        log(f"  [!] 服务器 {host}:{port} 无法连接: {e}")
        return False


def fmt_latency_stats(times_ms):
    if not times_ms:
        return {}
    s = sorted(times_ms)
    return {
        "count": len(times_ms),
        "mean": statistics.mean(s),
        "median": statistics.median(s),
        "min": min(s),
        "max": max(s),
        "p95": s[int(len(s) * 0.95) - 1],
        "p99": s[int(len(s) * 0.99) - 1],
    }


def bench_connection_rate(host, port, count=200):
    """TCP 连接建立速率"""
    log_separator(f"[1/4] TCP 连接建立速率测试 — {count} 次连接")

    times = []
    errors = 0
    for i in range(count):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        start = time.perf_counter()
        try:
            sock.connect((host, port))
            lat = (time.perf_counter() - start) * 1000
            times.append(lat)
        except Exception:
            errors += 1
        finally:
            try:
                sock.close()
            except Exception:
                pass
        if (i + 1) % 50 == 0:
            log(f"  进度: {i+1}/{count}")

    if not times:
        log("  [!] 全部连接失败，跳过")
        return []

    s = fmt_latency_stats(times)
    log(f"  成功: {s['count']}, 失败: {errors}")
    log(f"  平均连接耗时: {s['mean']:.2f} ms")
    log(f"  中位数:       {s['median']:.2f} ms")
    log(f"  P95:          {s['p95']:.2f} ms")
    log(f"  P99:          {s['p99']:.2f} ms")
    log(f"  最大:         {s['max']:.2f} ms")
    log(f"  最小:         {s['min']:.2f} ms")
    log(f"  连接速率:     {s['count'] / s['mean'] * 1000:.0f} 连接/秒")
    return times


def bench_request_latency(host, port, payload, count=100, concurrency=10):
    """请求-响应延迟 + 吞吐量"""
    log_separator(f"[2/4] 请求-响应延迟测试 — {count} 次, 并发={concurrency}")

    local_results = {"latencies": [], "errors": 0, "timeouts": 0}
    lock = threading.Lock()

    def worker():
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        sock.settimeout(5)
        try:
            sock.connect((host, port))
            pkt = build_packet(CMD_LOGIN, payload)
            start = time.perf_counter()
            sock.sendall(pkt)
            recv_response(sock)
            lat = (time.perf_counter() - start) * 1000
            with lock:
                local_results["latencies"].append(lat)
        except socket.timeout:
            with lock:
                local_results["timeouts"] += 1
        except Exception:
            with lock:
                local_results["errors"] += 1
        finally:
            try:
                sock.close()
            except Exception:
                pass

    total_start = time.perf_counter()
    with ThreadPoolExecutor(max_workers=concurrency) as pool:
        futures = [pool.submit(worker) for _ in range(count)]
        for i, f in enumerate(as_completed(futures)):
            if (i + 1) % 20 == 0:
                log(f"  进度: {i+1}/{count}")
        # 确保所有任务完成
        for f in futures:
            f.result()

    elapsed = time.perf_counter() - total_start
    lats = local_results["latencies"]

    if not lats:
        log("  [!] 无成功请求，跳过")
        return []

    s = fmt_latency_stats(lats)
    throughput = s["count"] / elapsed
    log(f"  成功: {s['count']}, 失败: {local_results['errors']}, 超时: {local_results['timeouts']}")
    log(f"  总耗时:       {elapsed:.2f} s")
    log(f"  吞吐量:       {throughput:.0f} req/s")
    log(f"  平均延迟:     {s['mean']:.2f} ms")
    log(f"  中位数:       {s['median']:.2f} ms")
    log(f"  P95:          {s['p95']:.2f} ms")
    log(f"  P99:          {s['p99']:.2f} ms")
    log(f"  最大延迟:     {s['max']:.2f} ms")
    log(f"  最小延迟:     {s['min']:.2f} ms")
    return lats, elapsed


def bench_concurrent_connections(host, port, target=500):
    """最大并发连接数"""
    log_separator(f"[3/4] 并发连接容量测试 — 目标 {target} 连接")

    connections = []
    start = time.perf_counter()
    success = 0
    for i in range(target):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        try:
            sock.connect((host, port))
            connections.append(sock)
            success += 1
            if success % 100 == 0:
                log(f"  已建立 {success} 连接...")
        except Exception as e:
            log(f"  连接失败 #{i}: {e}")
            break

    elapsed = time.perf_counter() - start
    log(f"  成功建立: {success} 连接")
    if elapsed > 0:
        log(f"  耗时:     {elapsed:.2f} s")
        log(f"  速率:     {success / elapsed:.0f} 连接/秒")

    for s in connections:
        try:
            s.close()
        except Exception:
            pass
    return success


def generate_report(args, conn_stats, req_stats, max_conns, server_alive, req_elapsed=None, concurrency=1):
    """生成 benchmark_result.md"""
    report = []
    report.append("# ChatServer 压测报告")
    report.append("")
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    report.append(f"- **测试时间**: {now}")
    report.append(f"- **目标地址**: `{args.host}:{args.port}`")
    report.append(f"- **服务器状态**: {'✅ 可达' if server_alive else '❌ 不可达'}")
    report.append("")

    if not server_alive:
        report.append("> ⚠ 服务器未启动或无法连接，请检查服务端后重试。")
        report.append("")
        report.append("---")
        report.append("")
        report.append("### 测试命令")
        report.append("```bash")
        report.append(f"python benchmark.py --host {args.host} --port {args.port}")
        report.append("```")
        report.append("")
        report_path = os.path.join(os.path.dirname(__file__), "benchmark_result.md")
        with open(report_path, "w", encoding="utf-8") as f:
            f.write("\n".join(report))
        return

    # 1. 连接速率
    report.append("---")
    report.append("")
    report.append("## 1. TCP 连接建立速率")
    report.append("")
    if conn_stats:
        report.append(f"| 指标 | 数值 |")
        report.append(f"|------|------|")
        report.append(f"| 测试次数 | {conn_stats['count']} |")
        report.append(f"| 平均耗时 | {conn_stats['mean']:.2f} ms |")
        report.append(f"| 中位数 | {conn_stats['median']:.2f} ms |")
        report.append(f"| P95 | {conn_stats['p95']:.2f} ms |")
        report.append(f"| P99 | {conn_stats['p99']:.2f} ms |")
        report.append(f"| 最大 | {conn_stats['max']:.2f} ms |")
        report.append(f"| 最小 | {conn_stats['min']:.2f} ms |")
        report.append(f"| 连接速率 | {conn_stats['count'] / conn_stats['mean'] * 1000:.0f} 连接/秒 |")
    else:
        report.append("无有效数据。")
    report.append("")

    # 2. 请求延迟
    report.append("---")
    report.append("")
    report.append("## 2. 请求-响应延迟 & 吞吐量")
    report.append("")
    if req_stats:
        report.append(f"| 指标 | 数值 |")
        report.append(f"|------|------|")
        report.append(f"| 测试次数 | {req_stats['count']} |")
        if req_elapsed and req_elapsed > 0:
            throughput = req_stats['count'] / req_elapsed
        else:
            throughput = 1000 / req_stats['mean'] * concurrency
        report.append(f"| 吞吐量 | {throughput:.0f} req/s |")
        report.append(f"| 平均延迟 | {req_stats['mean']:.2f} ms |")
        report.append(f"| 中位数 | {req_stats['median']:.2f} ms |")
        report.append(f"| P95 | {req_stats['p95']:.2f} ms |")
        report.append(f"| P99 | {req_stats['p99']:.2f} ms |")
        report.append(f"| 最大延迟 | {req_stats['max']:.2f} ms |")
        report.append(f"| 最小延迟 | {req_stats['min']:.2f} ms |")
    else:
        report.append("无有效数据。")
    report.append("")

    # 3. 并发连接
    report.append("---")
    report.append("")
    report.append("## 3. 并发连接容量")
    report.append("")
    report.append(f"| 指标 | 数值 |")
    report.append(f"|------|------|")
    report.append(f"| 最大并发连接数 | {max_conns} |")
    report.append("")

    # 汇总
    report.append("---")
    report.append("")
    report.append("## 汇总")
    report.append("")
    report.append(f"| 指标 | 数值 |")
    report.append(f"|------|------|")
    if conn_stats:
        report.append(f"| TCP 连接平均耗时 | {conn_stats['mean']:.2f} ms |")
    if req_stats:
        report.append(f"| 请求平均延迟 | {req_stats['mean']:.2f} ms |")
        if req_elapsed and req_elapsed > 0:
            throughput = req_stats['count'] / req_elapsed
        else:
            throughput = 1000 / req_stats['mean'] * concurrency
        report.append(f"| 吞吐量 | {throughput:.0f} req/s |")
    report.append(f"| 最大并发连接数 | {max_conns} |")
    report.append("")

    report.append("---")
    report.append("")
    report.append("### 测试命令")
    report.append("```bash")
    report.append(f"python benchmark.py --host {args.host} --port {args.port}")
    report.append("```")
    report.append("")

    report_path = os.path.join(os.path.dirname(__file__), "benchmark_result.md")
    content = "\n".join(report)
    with open(report_path, "w", encoding="utf-8") as f:
        f.write(content)
    log(f"\n  ✅ 测试报告已保存到 {report_path}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ChatServer 压测工具")
    parser.add_argument("--host", default="8.148.211.115", help="服务器地址（默认 127.0.0.1）")
    parser.add_argument("--port", type=int, default=8111, help="服务器端口（默认 8111）")
    parser.add_argument("--conn-count", type=int, default=200, help="连接测试次数（默认 200）")
    parser.add_argument("--req-count", type=int, default=100, help="请求测试次数（默认 100）")
    parser.add_argument("--concurrency", type=int, default=10, help="并发数（默认 10）")
    parser.add_argument("--max-conns", type=int, default=500, help="最大并发连接目标（默认 500）")
    args = parser.parse_args()

    print(f"{'=' * 60}")
    print(f"  ChatServer 压测工具")
    print(f"  时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"  目标: {args.host}:{args.port}")
    print(f"{'=' * 60}")

    server_alive = check_server(args.host, args.port)
    if not server_alive:
        log("\n  [!] 请先启动 ChatServer 服务端，再运行本工具。")
        generate_report(args, None, None, 0, False)
        sys.exit(1)

    login_payload = {"action": "login", "account": "18122778310", "password": "w123456789"}

    conn_stats = None
    req_stats = None
    req_elapsed = None
    max_conns = 0

    try:
        conn_times = bench_connection_rate(args.host, args.port, args.conn_count)
        conn_stats = fmt_latency_stats(conn_times) if conn_times else None
    except Exception as e:
        log(f"  [!] 连接速率测试异常: {e}")

    try:
        req_lats, req_elapsed = bench_request_latency(args.host, args.port, login_payload, args.req_count, args.concurrency)
        req_stats = fmt_latency_stats(req_lats) if req_lats else None
    except Exception as e:
        log(f"  [!] 请求延迟测试异常: {e}")

    try:
        max_conns = bench_concurrent_connections(args.host, args.port, args.max_conns)
    except Exception as e:
        log(f"  [!] 并发连接测试异常: {e}")

    # 汇总
    log()
    log_separator("测试完成")
    if conn_stats:
        log(f"  TCP 连接平均耗时:  {conn_stats['mean']:.2f} ms")
    if req_stats:
        log(f"  请求平均延迟:      {req_stats['mean']:.2f} ms")
        if req_elapsed and req_elapsed > 0:
            throughput = req_stats['count'] / req_elapsed
        else:
            throughput = 1000 / req_stats['mean'] * args.concurrency
        log(f"  吞吐量:            {throughput:.0f} req/s")
    log(f"  最大并发连接数:    {max_conns}")

    generate_report(args, conn_stats, req_stats, max_conns, True, req_elapsed, args.concurrency)
