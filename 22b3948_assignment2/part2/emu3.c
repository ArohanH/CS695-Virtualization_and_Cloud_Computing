#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <linux/kvm.h>

/* CR0 bits */
#define CR0_PE 1u
#define CR0_MP (1U << 1)
#define CR0_EM (1U << 2)
#define CR0_TS (1U << 3)
#define CR0_ET (1U << 4)
#define CR0_NE (1U << 5)
#define CR0_WP (1U << 16)
#define CR0_AM (1U << 18)
#define CR0_NW (1U << 29)
#define CR0_CD (1U << 30)
#define CR0_PG (1U << 31)

/* CR4 bits */
#define CR4_VME 1
#define CR4_PVI (1U << 1)
#define CR4_TSD (1U << 2)
#define CR4_DE (1U << 3)
#define CR4_PSE (1U << 4)
#define CR4_PAE (1U << 5)
#define CR4_MCE (1U << 6)
#define CR4_PGE (1U << 7)
#define CR4_PCE (1U << 8)
#define CR4_OSFXSR (1U << 8)
#define CR4_OSXMMEXCPT (1U << 10)
#define CR4_UMIP (1U << 11)
#define CR4_VMXE (1U << 13)
#define CR4_SMXE (1U << 14)
#define CR4_FSGSBASE (1U << 16)
#define CR4_PCIDE (1U << 17)
#define CR4_OSXSAVE (1U << 18)
#define CR4_SMEP (1U << 20)
#define CR4_SMAP (1U << 21)

#define EFER_SCE 1
#define EFER_LME (1U << 8)
#define EFER_LMA (1U << 10)
#define EFER_NXE (1U << 11)

/* 32-bit page directory entry bits */
#define PDE32_PRESENT 1
#define PDE32_RW (1U << 1)
#define PDE32_USER (1U << 2)
#define PDE32_PS (1U << 7)

/* 64-bit page * entry bits */
#define PDE64_PRESENT 1
#define PDE64_RW (1U << 1)
#define PDE64_USER (1U << 2)
#define PDE64_ACCESSED (1U << 5)
#define PDE64_DIRTY (1U << 6)
#define PDE64_PS (1U << 7)
#define PDE64_G (1U << 8)

#define PRODUCER_PORT_FOR_ADDRESS 0xE9
#define CONSUMER_PORT_FOR_ADDRESS 0xEA
#define CONSUMER_PORT_FOR_DATA 0xEB
#define PRODUCER_PORT_FOR_DATA 0xEC
#define NUM_ITEMS 5

uint64_t producer_base_address = 0;
uint64_t consumer_base_address = 0;


struct vm
{
	int dev_fd;
	int vm_fd;
	char *mem;
};

struct vcpu
{
	int vcpu_fd;
	struct kvm_run *kvm_run;
};

void vm_init(struct vm *vm, size_t mem_size)
{
	int kvm_version;
	struct kvm_userspace_memory_region memreg;

	vm->dev_fd = open("/dev/kvm", O_RDWR);
	if (vm->dev_fd < 0)
	{
		perror("open /dev/kvm");
		exit(1);
	}

	kvm_version = ioctl(vm->dev_fd, KVM_GET_API_VERSION, 0);
	if (kvm_version < 0)
	{
		perror("KVM_GET_API_VERSION");
		exit(1);
	}

	if (kvm_version != KVM_API_VERSION)
	{
		fprintf(stderr, "Got KVM api version %d, expected %d\n",
				kvm_version, KVM_API_VERSION);
		exit(1);
	}

	vm->vm_fd = ioctl(vm->dev_fd, KVM_CREATE_VM, 0);
	if (vm->vm_fd < 0)
	{
		perror("KVM_CREATE_VM");
		exit(1);
	}

	if (ioctl(vm->vm_fd, KVM_SET_TSS_ADDR, 0xfffbd000) < 0)
	{
		perror("KVM_SET_TSS_ADDR");
		exit(1);
	}

	vm->mem = mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
				   MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	if (vm->mem == MAP_FAILED)
	{
		perror("mmap mem");
		exit(1);
	}

	madvise(vm->mem, mem_size, MADV_MERGEABLE);

	memreg.slot = 0;
	memreg.flags = 0;
	memreg.guest_phys_addr = 0;
	memreg.memory_size = mem_size;
	memreg.userspace_addr = (unsigned long)vm->mem;
	if (ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &memreg) < 0)
	{
		perror("KVM_SET_USER_MEMORY_REGION");
		exit(1);
	}
}

void vcpu_init(struct vm *vm, struct vcpu *vcpu)
{
	int vcpu_mmap_size;

	vcpu->vcpu_fd = ioctl(vm->vm_fd, KVM_CREATE_VCPU, 0);
	if (vcpu->vcpu_fd < 0)
	{
		perror("KVM_CREATE_VCPU");
		exit(1);
	}

	vcpu_mmap_size = ioctl(vm->dev_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
	if (vcpu_mmap_size <= 0)
	{
		perror("KVM_GET_VCPU_MMAP_SIZE");
		exit(1);
	}

	vcpu->kvm_run = mmap(NULL, vcpu_mmap_size, PROT_READ | PROT_WRITE,
						 MAP_SHARED, vcpu->vcpu_fd, 0);
	if (vcpu->kvm_run == MAP_FAILED)
	{
		perror("mmap kvm_run");
		exit(1);
	}
}

/* Modify this function to complete part 2.3 */
int run_vm(struct vm *vm1, struct vm *vm2, struct vcpu *vcpu1, struct vcpu *vcpu2, size_t sz)
{
	struct kvm_regs regs1;
	//struct vm *vm = NULL;
	//struct vcpu *vcpu = NULL;
	uint64_t memval1 = 0;
	struct kvm_regs regs2;
	uint64_t memval2 = 0;
    int produced[NUM_ITEMS];
    int consumed[NUM_ITEMS];

    for (;;) {
        /* --- Run the Producer VM --- */
        if (ioctl(vcpu1->vcpu_fd, KVM_RUN, 0) < 0) {
            perror("KVM_RUN on producer");
            exit(1);
        }
        //sleep(1);

        switch (vcpu1->kvm_run->exit_reason)
		{
		case KVM_EXIT_HLT:
			goto check1;
		case KVM_EXIT_IO:
            if (vcpu1->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                vcpu1->kvm_run->io.port == PRODUCER_PORT_FOR_DATA)
            {
                memcpy(produced, vm1->mem + producer_base_address, NUM_ITEMS * sizeof(int));

                printf("VMFD: %d Produced Values:", vm1->vm_fd);
                for (int i = 0; i < NUM_ITEMS; i++) {
                    printf(" %d", produced[i]);
                }
                printf("\n");
				fflush(stdout);

                memcpy(vm2->mem + consumer_base_address, produced, NUM_ITEMS * sizeof(int));
				break;				
            } 
			else if(vcpu1->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                vcpu1->kvm_run->io.port == PRODUCER_PORT_FOR_ADDRESS){
				char *p = (char *)vcpu1->kvm_run;
				uintptr_t guest_ptr = *(uintptr_t *)(p + vcpu1->kvm_run->io.data_offset);
				char *addr = (char *) guest_ptr;
				struct kvm_translation translation;
				translation.linear_address = (uint64_t)addr;
				if (ioctl(vcpu1->vcpu_fd, KVM_TRANSLATE, &translation) == -1) {
					perror("ioctl KVM_TRANSLATE failed");
					exit(EXIT_FAILURE);
				}
				if (!translation.valid) {
					printf("Invalid Guest Virtual Address: 0x%p\n", (void *) addr);
					fflush(stdout);
				}
				else{
					producer_base_address = translation.physical_address;
				}
				break;
			} else {
                fprintf(stderr, "Producer: Unexpected I/O port: 0x%x\n",
                        vcpu1->kvm_run->io.port);
                exit(1);
            }
        default:
            fprintf(stderr, "Producer: Unexpected exit_reason %d\n",
                    vcpu1->kvm_run->exit_reason);
            exit(1);
        }

        /* --- Run the Consumer VM --- */
        if (ioctl(vcpu2->vcpu_fd, KVM_RUN, 0) < 0) {
            perror("KVM_RUN on consumer");
            exit(1);
        }
        //sleep(1);


        switch (vcpu2->kvm_run->exit_reason)
		{
		case KVM_EXIT_HLT:
			goto check2;
		case KVM_EXIT_IO:
            if (vcpu2->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                vcpu2->kvm_run->io.port == CONSUMER_PORT_FOR_DATA)
            {
				char *p = (char *)vcpu2->kvm_run;
				uintptr_t guest_ptr = *(uintptr_t *)(p + vcpu2->kvm_run->io.data_offset);
				char *addr = (char *) guest_ptr;
				struct kvm_translation translation;
				translation.linear_address = (uint64_t)addr;
				if (ioctl(vcpu2->vcpu_fd, KVM_TRANSLATE, &translation) == -1) {
					perror("ioctl KVM_TRANSLATE failed");
					exit(EXIT_FAILURE);
				}
				if (!translation.valid) {
					printf("Invalid Guest Virtual Address: 0x%p\n", (void *) addr);
					fflush(stdout);
				}
				else{
					memcpy(consumed, vm2->mem + translation.physical_address, NUM_ITEMS * sizeof(int));
					printf("VMFD: %d Consumed Values:", vm2->vm_fd);
					for (int i = 0; i < NUM_ITEMS; i++) {
						printf(" %d", consumed[i]);
					}
					printf("\n");
					fflush(stdout);
				}
				break;
			}
			else if(vcpu2->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
				vcpu2->kvm_run->io.port == CONSUMER_PORT_FOR_ADDRESS){
				char *p = (char *)vcpu2->kvm_run;
				uintptr_t guest_ptr = *(uintptr_t *)(p + vcpu2->kvm_run->io.data_offset);
				char *addr = (char *) guest_ptr;
				struct kvm_translation translation;
				translation.linear_address = (uint64_t)addr;
				if (ioctl(vcpu2->vcpu_fd, KVM_TRANSLATE, &translation) == -1) {
					perror("ioctl KVM_TRANSLATE failed");
					exit(EXIT_FAILURE);
				}
				if (!translation.valid) {
					printf("Invalid Guest Virtual Address: 0x%p\n", (void *) addr);
					fflush(stdout);
				}
				else{
					consumer_base_address = translation.physical_address;
				}
				break;
            } else {
                fprintf(stderr, "Consumer: Unexpected I/O port: 0x%x\n",
                        vcpu2->kvm_run->io.port);
                exit(1);
            }
        default:
            fprintf(stderr, "Consumer: Unexpected exit_reason %d\n",
                    vcpu2->kvm_run->exit_reason);
            exit(1);
        }
    }


check1:
	if (ioctl(vcpu1->vcpu_fd, KVM_GET_REGS, &regs1) < 0)
	{
		perror("KVM_GET_REGS");
		exit(1);
	}

	if (regs1.rax != 42)
	{
		printf("Wrong result: {E,R,}AX is %lld\n", regs1.rax);
		return 0;
	}

	memcpy(&memval1, &vm1->mem[0x400], sz);
	if (memval1 != 42)
	{
		printf("Wrong result: memory at 0x400 is %lld\n",
			   (unsigned long long)memval1);
		return 0;
	}
	goto end;

check2:
	if (ioctl(vcpu2->vcpu_fd, KVM_GET_REGS, &regs2) < 0)
	{
		perror("KVM_GET_REGS");
		exit(1);
	}

	if (regs2.rax != 42)
	{
		printf("Wrong result: {E,R,}AX is %lld\n", regs2.rax);
		return 0;
	}

	memcpy(&memval2, &vm2->mem[0x400], sz);
	if (memval2 != 42)
	{
		printf("Wrong result: memory at 0x400 is %lld\n",
			   (unsigned long long)memval2);
		return 0;
	}
	goto end;

end:
	return 1;
}

static void setup_protected_mode(struct kvm_sregs *sregs)
{
	struct kvm_segment seg = {
		.base = 0,
		.limit = 0xffffffff,
		.selector = 1 << 3,
		.present = 1,
		.type = 11, /* Code: execute, read, accessed */
		.dpl = 0,
		.db = 1,
		.s = 1, /* Code/data */
		.l = 0,
		.g = 1, /* 4KB granularity */
	};

	sregs->cr0 |= CR0_PE; /* enter protected mode */

	sregs->cs = seg;

	seg.type = 3; /* Data: read/write, accessed */
	seg.selector = 2 << 3;
	sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}

extern const unsigned char guest3a[], guest3a_end[];
extern const unsigned char guest3b[], guest3b_end[];

int run_protected_mode1(struct vm *vm, struct vcpu *vcpu)
{
	struct kvm_sregs sregs;
	struct kvm_regs regs;

	if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0)
	{
		perror("KVM_GET_SREGS");
		exit(1);
	}

	setup_protected_mode(&sregs);

	if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0)
	{
		perror("KVM_SET_SREGS");
		exit(1);
	}

	memset(&regs, 0, sizeof(regs));
	/* Clear all FLAGS bits, except bit 1 which is always set. */
	regs.rflags = 2;
	regs.rip = 0;
	regs.rsp = 2 << 20;

	if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &regs) < 0)
	{
		perror("KVM_SET_REGS");
		exit(1);
	}

	memcpy(vm->mem, guest3a, guest3a_end - guest3a);
	printf("VMFD: %d Loaded Program with size: %ld\n", vm->vm_fd, guest3a_end - guest3a);
	return 0;
}

int run_protected_mode2(struct vm *vm, struct vcpu *vcpu)
{
	struct kvm_sregs sregs;
	struct kvm_regs regs;

	if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0)
	{
		perror("KVM_GET_SREGS");
		exit(1);
	}

	setup_protected_mode(&sregs);

	if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0)
	{
		perror("KVM_SET_SREGS");
		exit(1);
	}

	memset(&regs, 0, sizeof(regs));
	/* Clear all FLAGS bits, except bit 1 which is always set. */
	regs.rflags = 2;
	regs.rip = 0;
	regs.rsp = 2 << 20;

	if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &regs) < 0)
	{
		perror("KVM_SET_REGS");
		exit(1);
	}

	memcpy(vm->mem, guest3b, guest3b_end - guest3b);
	printf("VMFD: %d Loaded Program with size: %ld\n", vm->vm_fd, guest3b_end - guest3b);
	return 0;
}

int main()
{
	struct vm vm1, vm2;
	struct vcpu vcpu1, vcpu2;

	vm_init(&vm1, 0x200000);
	vm_init(&vm2, 0x200000);
	vcpu_init(&vm1, &vcpu1);
	vcpu_init(&vm2, &vcpu2);
	run_protected_mode1(&vm1, &vcpu1);
	run_protected_mode2(&vm2, &vcpu2);
	return run_vm(&vm1, &vm2, &vcpu1, &vcpu2, 4);
}
