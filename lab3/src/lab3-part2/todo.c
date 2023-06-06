// func == 1
static void scan_vma(void)
{
    struct mm_struct *mm;
    printk("func == 1, %s\n", __func__);
    struct mm_struct *mm = get_task_mm(my_task_info.task);
    if (mm)
    {
        // TODO :����VMA��VMA�ĸ�����¼��my_task_info��vma_cnt������
        struct vm_area_struct *vm_area = mm->mmap;
        while (vm_area != NULL) { // ����vm_area_struct�ṹ��
            my_task_info.vma_cnt++;
            vm_area = vm_area->vm_next; // ָ����һ��vm_area_struct�ṹ��
        }
        mmput(mm);
    }
}

// func == 2
static void print_mm_active_info(void)
{
    int first = 1;
    struct mm_struct *mm;
    unsigned long virt_addr;
    printk("func == 2, %s\n", __func__);
    // TODO : 1. ����VMA��������VMA�������ַ�õ���Ӧ��struct page�ṹ�壨ʹ��mfollow_page������
    // struct page *page = mfollow_page(vma, virt_addr, FOLL_GET);
    // unsigned int unused_page_mask;
    // struct page *page = mfollow_page_mask(vma, virt_addr, FOLL_GET, &unused_page_mask);
    // TODO : 2. ʹ��page_referenced��Ծҳ���Ƿ񱻷��ʣ����������ʵ�ҳ�������ַд���ļ���
    // kernel v5.13.0-40��֮��ɳ���
    // unsigned long vm_flags;
    // int freq = mpage_referenced(page, 0, (struct mem_cgroup *)(page->memcg_data), &vm_flags);
    // kernel v5.9.0
    // unsigned long vm_flags;
    // int freq = mpage_referenced(page, 0, page->mem_cgroup, &vm_flags);
    mm = get_task_mm(my_task_info.task);
    if (mm) {
        struct vm_area_struct *vm_area = mm->mmap; // ��ȡmm_struct�ṹ���е�vm_area_struct�ṹ��
        while (vm_area) { // ����vm_area_struct�ṹ��
            pr_info("%lx", vm_area->vm_flags); // ��ӡVMA��־��Ϣ
            virt_addr = vm_area->vm_start;
            while (virt_addr != vm_area->vm_end) { // ����VMA�е������ַ
                struct page *page = mfollow_page(vm_area, virt_addr, FOLL_GET | FOLL_TOUCH); 
                // ��ȡ�����ַ��Ӧ��struct page�ṹ��
                if (!IS_ERR_OR_NULL(page)) {
                    // ���ҳ�汻���ʣ����������ַת��Ϊ�ַ�����������д���ļ���
                    //ʹ��һ���ַ���������`str_buf`���洢�����ַ���ַ�����ʾ��
                    //ͨ��`sprintf()`������ÿ��ҳ��������ַ׷�ӵ�`str_buf`�У�
                    //Ȼ�����`write_to_file()`������`str_buf`д���ļ���
                    unsigned long vm_flags; // ���ڴ洢ҳ��ı�־
                    int freq = mpage_referenced(page, 0, (struct mem_cgroup *)(page->memcg_data), &vm_flags); 
                    // �ж�ҳ���Ƿ񱻷���
                    if (freq != 0) {
                        if (first) { // �ж��Ƿ��ǵ�һ��д���ļ�
                            sprintf(str_buf, "%lu", page_to_pfn(page)); 
                            // ��ҳ�������ҳ֡��pfnת��Ϊ�ַ�����������洢�� str_buf ��������
                            first = 0; // ��first��Ϊ0, ���ǵ�һ����
                        } else {
                            sprintf(str_buf, ",%lu", page_to_pfn(page)); 
                            // ��ҳ�������ҳ֡��ת��Ϊ�ַ�������׷�ӵ� str_buf �������У��Զ��ŷָ���������ַ
                        }
                        write_to_file(str_buf, strlen(str_buf)); 
                    }
                }
                virt_addr += PAGE_SIZE;
            }
            vm_area = vm_area->vm_next;
        }
        sprintf(str_buf, "\n");
        write_to_file(str_buf, strlen(str_buf));
        mmput(mm);
    }
}

static unsigned long virt2phys(struct mm_struct *mm, unsigned long virt)
{
    struct page *page = NULL;
    // TODO : �༶ҳ�������pgd->pud->pmd->pte��Ȼ���pte��page�����õ�pfn
    pgd_t *pgd = pgd_offset(mm, virt); // 1��ҳ��
    p4d_t *p4d = p4d_offset(pgd, virt); // 4��ҳ��
    pud_t *pud = pud_offset(p4d, virt); // 3��ҳ��
    pmd_t *pmd = pmd_offset(pud, virt); // 2��ҳ��
    pte_t *pte = pte_offset_kernel(pmd, virt); 
    
    if (pte_present(*pte)) // �ж�pte�Ƿ����
    {
        page = pte_page(*pte); // ��ȡpte��Ӧ��page
        if (page)
        {
            return page_to_phys(page); // ��ȡpage��Ӧ�������ַ
        }
    }
    
    pr_err("func: %s page is NULL\n", __func__);
    return 0;
}

// func = 3
static void traverse_page_table(struct task_struct *task)
{
    struct mm_struct *mm;
    printk("func == 3, %s\n", __func__);
    struct mm_struct *mm = get_task_mm(my_task_info.task);
    if (mm)
    {
        // TODO :����VMA������PAGE_SIZEΪ�����������VMA�е������ַ��Ȼ�����ҳ�����
        // record_two_data(virt_addr, virt2phys(task->mm, virt_addr));
        struct vm_area_struct *vm_area = mm->mmap;
        while (vm_area) {
            unsigned long virt_addr = vm_area->vm_start;
            while (virt_addr != vm_area->vm_end) { // ����VMA�е������ַ
                struct page *page = mfollow_page(vm_area, virt_addr, FOLL_GET); // ��ȡ�����ַ��Ӧ��struct page�ṹ��
                if (!IS_ERR_OR_NULL(page)) { // �ж�page�Ƿ����
                    sprintf(str_buf, "0x%lx--0x%lx--0x%lx\n", virt_addr, page_to_pfn(page),virt2phys(mm,virt_addr)); 
                    // �������ַ��PFN�������ַд���ļ� ��ʽ��Ϊ�ַ���
                    write_to_file(str_buf, strlen(str_buf));
                }
                virt_addr += PAGE_SIZE; // �����ַ����PAGE_SIZE
            }
            vm_area = vm_area->vm_next;
        }
        mmput(mm); // �ͷ�mm_struct�ṹ�� 
    }
    else
    {
        pr_err("func: %s mm_struct is NULL\n", __func__);
    }
}

// func == 4 ���� func == 5
static void print_seg_info(void)
{
    struct mm_struct *mm;
    unsigned long addr;
    unsigned long start,end;
    struct vm_area_struct *vm_area;
    printk("func == 4 or func == 5, %s\n", __func__);
    mm = get_task_mm(my_task_info.task);
    if (mm == NULL)
    {
        pr_err("mm_struct is NULL\n");
        return;
    }

    if (ktest_func == 4) {
        start = mm->start_data; // ���ݶε���ʼ��ַ
        end = mm->end_data;
    } else {
        start = mm->start_code; // ����ε���ʼ��ַ 
        end = mm->end_code;
    }
    vm_area = mm->mmap;
    // TODO: �������ݶλ��ߴ���ε���ʼ��ַ����ֹ��ַ�õ����е�ҳ�棬Ȼ���ӡҳ�����ݵ��ļ���
    // ��ʾ������ʹ�� follow_page �����õ������ַ��Ӧ�� page��Ȼ��ʹ�� addr = kmap_atomic(page) �õ�����ֱ��
    // ���ʵ������ַ��Ȼ�����ʹ�� memcpy ���������ݶλ����ο�����ȫ�ֱ��� buf ����д�뵽�ļ���
    // ע�⣺ʹ�� kmap_atomic(page) ��������Ҫʹ�� kunmap_atomic(addr) �����ͷŲ���
    // ��ȷ��������������ʵ���ṩ�� workload����һ�������ݶ�Ӧ�û��ӡ�� char *trace_data��
    // static char global_data[100] �� char hamlet_scene1[8276] �����ݡ�

    while (vm_area) {
        unsigned long virt_addr = vm_area->vm_start;
        while (virt_addr < vm_area->vm_end) {
            if (start <= virt_addr && end >= virt_addr + PAGE_SIZE) { // ��ǰҳ��ȫ���������ݶλ�������
                struct page *page = mfollow_page(vm_area, virt_addr, FOLL_GET); // ��ȡ�����ַ��Ӧ��struct page�ṹ��
                if (!IS_ERR_OR_NULL(page)) {
                    addr = kmap_atomic(page); // ��ȡ�����ַ��Ӧ�������ַ
                    memcpy(buf, addr, PAGE_SIZE); // ��PAGE_SIZE��С�����ݴ�addr������buf��
                    kunmap_atomic(addr); // �ͷ�addr
                    write_to_file(buf, PAGE_SIZE); // ��buf�е�PAGE_SIZE��С������д���ļ�
                }
            }
            else if (end >= virt_addr && end <= virt_addr + PAGE_SIZE) { // ��ǰҳ�Ĳ������������ݶλ�������
                struct page *page = mfollow_page(vm_area, virt_addr, FOLL_GET); 
                if (!IS_ERR_OR_NULL(page)) {
                    addr = kmap_atomic(page);
                    memcpy(buf, addr, end - virt_addr); // ��end - virt_addr��С�����ݴ�addr������buf��
                    kunmap_atomic(addr);
                    write_to_file(buf, end - virt_addr);
                }
            }
            else if (start >= virt_addr && start <= virt_addr + PAGE_SIZE) { // ��ǰҳ�Ĳ������������ݶλ�������
                struct page *page = mfollow_page(vm_area, virt_addr, FOLL_GET);
                if (!IS_ERR_OR_NULL(page)) {
                    addr = kmap_atomic(page);
                    memcpy(buf, addr + start - virt_addr, virt_addr + PAGE_SIZE - start);
                    kunmap_atomic(addr);
                    write_to_file(buf, virt_addr + PAGE_SIZE - start);
                }
            }
            virt_addr += PAGE_SIZE;
        }
        vm_area = vm_area->vm_next;
    }
    mmput(mm); // �ͷ�mm_struct�ṹ��
}

