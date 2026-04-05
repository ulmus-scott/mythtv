import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';

import { JobQCommands } from 'src/app/services/interfaces/setup.interface';
import { SetupService } from 'src/app/services/setup.service';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';
import { TranslateDirective, TranslatePipe } from '@ngx-translate/core';

import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { SettingsComponent } from '../general-settings.component';

@Component({
    selector: 'app-jobqueue-commands',
    templateUrl: './jobqueue-commands.component.html',
    styleUrls: ['./jobqueue-commands.component.css'],
    imports: [FormsModule, CardModule, SharedModule, TranslatePipe, MessageModule, ButtonModule, TranslateDirective]
})
export class JobqueueCommandsComponent implements OnInit, AfterViewInit {

    @ViewChild("jobqcommands") currentForm!: NgForm;
    @Input() parent!: SettingsComponent;
    @Input() tabIndex!: number;

    JobQCommandsData!: JobQCommands;
    items: number[] = [0, 1, 2, 3];

    constructor(public setupService: SetupService) {
        this.JobQCommandsData = this.setupService.getJobQCommands();
    }

    ngOnInit(): void {
        this.parent.children[this.tabIndex] = this;
    }

    dirty() {
        return this.currentForm.dirty;
    }

    ngAfterViewInit() {
    }

    markPristine() {
        setTimeout(() => {
            this.currentForm.form.markAsPristine();
            this.parent.showDirty();
        }, 100);
    }

    saveForm() {
        this.setupService.saveJobQCommands(this.currentForm);
    }

}
