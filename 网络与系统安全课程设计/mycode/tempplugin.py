# -*- coding: utf-8 -*-

import idaapi
import idc
import idautils
import re

# --- 统一签名数据库 (Unified Signature Database with Risk Scoring) ---
# risk: 1 (低) - 10 (高)
SIGS = {
    # --- 漏洞特征 (Vulnerability Signatures) ---
    "gets":         {"type": "Vulnerability", "label": "Stack Overflow", "risk": 10, "description": "函数 'gets' 不检查边界，极易引发缓冲区溢出，是应被禁用的危险函数。"},
    "strcpy":       {"type": "Vulnerability", "label": "Stack Overflow", "risk": 7,  "description": "函数 'strcpy' 不检查边界，若源字符串长于目标缓冲区，将导致缓冲区溢出。"},
    "sprintf":      {"type": "Vulnerability", "label": "Stack Overflow", "risk": 7,  "description": "函数 'sprintf' 不检查边界，若格式化后的字符串长于目标缓冲区，将导致溢出。"},
    "scanf":        {"type": "Vulnerability", "label": "Stack Overflow", "risk": 6,  "description": "使用 '%s' 等格式符时，若无长度限制，'scanf' 会持续写入直到遇到空白符，易导致溢出。"},
    "memcpy":       {"type": "Vulnerability", "label": "Heap/Stack Overflow", "risk": 5, "description": "内存拷贝函数，若拷贝长度不受控制或计算错误，可能导致堆或栈的缓冲区溢出。"},
    "free":         {"type": "Vulnerability", "label": "Memory Corruption", "risk": 3, "description": "涉及内存释放，可能与 Use-After-Free 或 Double Free 相关，需结合上下文分析。"},
    
    # --- 恶意行为特征 (Malware Signatures) ---
    "system":       {"type": "Malicious", "label": "Command Execution", "risk": 9,  "description": "通过 'system' 执行外部命令，是反弹Shell、执行恶意脚本的常用手段。"},
    "popen":        {"type": "Malicious", "label": "Command Execution", "risk": 9,  "description": "通过 'popen' 执行外部命令并创建管道，可用于执行指令并获取回显。"},
    "exec":         {"type": "Malicious", "label": "Process Manipulation", "risk": 8, "description": "系列函数可执行新程序，常被用于加载或替换为恶意代码。"},
    "socket":       {"type": "Malicious", "label": "Network Activity", "risk": 4,  "description": "创建网络套接字，是所有网络通信（包括后门、数据窃取）的基础。"},
    "bind":         {"type": "Malicious", "label": "Network Listener", "risk": 5,  "description": "将套接字绑定到特定端口，是监听服务（如后门）的必要步骤。"},
    "listen":       {"type": "Malicious", "label": "Network Listener", "risk": 5,  "description": "在已绑定的端口上进行监听，等待外部连接。"},
    "setenforce":   {"type": "Malicious", "label": "Disable Security", "risk": 8,  "description": "企图关闭 SELinux，旨在降低系统安全防护等级。"},
    "remove":       {"type": "Malicious", "label": "File Deletion", "risk": 6, "description": "删除文件，可能用于破坏系统或清除痕迹。"},
    
    # --- 高风险字符串 (High-Risk String Literals) ---
    "/etc/passwd":  {"type": "Malicious", "label": "Credential Access", "risk": 9, "description": "引用系统用户账户文件，可能意图窃取用户信息。"},
    "/etc/shadow":  {"type": "Malicious", "label": "Credential Access", "risk": 10, "description": "引用系统密码哈希文件，是窃取用户凭据的严重恶意行为。"},
    "/bin/sh":      {"type": "Malicious", "label": "Shell Command", "risk": 7, "description": "引用 '/bin/sh'，通常用于执行shell命令，是命令注入或反弹shell的前兆。"},
    "nc -l":        {"type": "Malicious", "label": "Backdoor Creation", "risk": 8, "description": "使用 netcat 创建监听器，是开启后门的典型特征。"},
    "netcat":       {"type": "Malicious", "label": "Backdoor Creation", "risk": 8, "description": "使用 netcat，可能用于文件传输、端口扫描或创建后门。"}
}

# 用于执行命令的函数关键字
CMD_EXEC_FUNCS = {"system", "popen", "exec"}

# 全局发现列表，用于存储所有风险项
g_findings = []

def record_finding(address, risk, label, keyword, evidence, description):
    """
    记录一个发现，并添加到全局列表和IDA注释中
    """
    finding = {
        "address": address,
        "risk": risk,
        "label": label,
        "keyword": keyword,
        "evidence": evidence.strip(),
        "description": description
    }
    
    # 防止重复记录同一地址的同一风险
    for existing_finding in g_findings:
        if existing_finding['address'] == address and existing_finding['keyword'] == keyword:
            return
            
    g_findings.append(finding)
    
    # 在IDA反汇编窗口中添加注释
    comment = "[RISK %d] %s: %s" % (risk, label, keyword)
    existing_comment = idc.get_cmt(address, 0)
    if not existing_comment or "[RISK" not in existing_comment:
        idc.set_cmt(address, comment, 0)
    print("  + Found risk at 0x{:X}: {}".format(address, comment))


def analyze_direct_calls():
    """
    分析函数中的直接调用和指令，类似旧版逻辑但更精确
    """
    print("\n[Phase 1] Analyzing direct function calls and instructions...")
    for func_addr in idautils.Functions():
        # 使用IDA的标志来过滤库函数，比硬编码白名单更可靠
        func_flags = idc.get_func_flags(func_addr)
        if func_flags & idc.FUNC_LIB:
            continue

        func_name = idc.get_func_name(func_addr)
        func_end = idc.find_func_end(func_addr)
        
        for instr_addr in idautils.Heads(func_addr, func_end):
            disasm_line = idc.generate_disasm_line(instr_addr, 0).lower()
            
            for keyword, sig_data in SIGS.items():
                if re.search(r'\b' + re.escape(keyword) + r'\b', disasm_line):
                    record_finding(instr_addr, sig_data['risk'], sig_data['label'], keyword, disasm_line, sig_data['description'])


def analyze_string_xrefs_and_calls():
    """
    [新功能] 分析高风险字符串的交叉引用，并检查其上下文是否有危险函数调用
    """
    print("\n[Phase 2] Analyzing cross-references of high-risk strings...")
    for s in idautils.Strings():
        str_content = str(s).lower()
        
        for keyword, sig_data in SIGS.items():
            # 只关心作为字符串的签名
            if "/" not in keyword and "nc" not in keyword:
                continue

            if keyword in str_content:
                # 找到了一个高风险字符串，现在检查它的用途
                for xref in idautils.XrefsTo(s.ea):
                    xref_addr = xref.frm
                    # 检查引用点附近（5条指令内）是否有命令执行函数
                    for i in range(5):
                        instr_addr = idc.next_head(xref_addr + i)
                        if instr_addr == idaapi.BADADDR:
                            break
                        
                        disasm_line = idc.generate_disasm_line(instr_addr, 0).lower()
                        for cmd_func in CMD_EXEC_FUNCS:
                            if cmd_func in disasm_line:
                                risk = 10 # 确认组合，风险等级最高
                                label = "Confirmed Command Injection"
                                evidence = '"{}" used by call to "{}"'.format(str(s), cmd_func)
                                description = "高风险字符串 '{}' 被传递给命令执行函数 '{}'，这极有可能是恶意行为。".format(str(s), cmd_func)
                                record_finding(xref_addr, risk, label, cmd_func, evidence, description)
                                break


def report_findings():
    """
    格式化并打印所有发现的风险项，按风险等级排序
    """
    if not g_findings:
        print("\n\n" + "="*60)
        print("    SCAN COMPLETE: No specific risks identified.")
        print("="*60)
        return
        
    # 按风险分值从高到低排序
    sorted_findings = sorted(g_findings, key=lambda x: x['risk'], reverse=True)
    
    print("\n\n" + "="*60)
    print("            SECURITY ANALYSIS REPORT")
    print("="*60)
    print("Total risks found: {}".format(len(sorted_findings)))
    
    for finding in sorted_findings:
        print("\n------------------------------------------------------------")
        print("  - RISK LEVEL: {}".format(finding['risk']))
        print("  - ADDRESS:    0x{:X}".format(finding['address']))
        print("  - LABEL:      {}".format(finding['label']))
        print("  - KEYWORD:    {}".format(finding['keyword']))
        print("  - EVIDENCE:   {}".format(finding['evidence']))
        print("  - ANALYSIS:   {}".format(finding['description']))
    print("\n------------------------------------------------------------")


def main():
    """
    主函数
    """
    # 清空上一次的运行结果
    global g_findings
    g_findings = []
    
    print("Starting enhanced security scan for vulnerabilities and malware patterns...")
    
    # 执行两个阶段的分析
    analyze_direct_calls()
    analyze_string_xrefs_and_calls()
    
    # 生成最终报告
    report_findings()
    
    print("\n\nScan finished. Reminder: This report is a static analysis reference. All findings require manual confirmation.")


if __name__ == "__main__":
    main()