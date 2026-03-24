import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';

import { MythService } from 'src/app/services/myth.service';
import { SetupService } from 'src/app/services/setup.service';
import { TranslateModule } from '@ngx-translate/core';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';
import { NgIf } from '@angular/common';
import { CheckboxModule } from 'primeng/checkbox';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { SettingsComponent } from '../general-settings.component';

@Component({
    selector: 'app-jobqueue-global',
    templateUrl: './jobqueue-global.component.html',
    styleUrls: ['./jobqueue-global.component.css'],
    standalone: true,
    imports: [FormsModule, CardModule, SharedModule, CheckboxModule, NgIf, MessageModule, ButtonModule, TranslateModule]
})
export class JobqueueGlobalComponent implements OnInit, AfterViewInit {

    @ViewChild("jobqglobal") currentForm!: NgForm;
    @Input() parent!: SettingsComponent;
    @Input() tabIndex!: number;

    successCount = 0;
    errorCount = 0;
    JobsRunOnRecordHost = false;
    AutoCommflagWhileRecording = false;
    JobQueueCommFlagCommand = "mythcommflag";
    JobQueueTranscodeCommand = "mythtranscode";
    AutoTranscodeBeforeAutoCommflag = false;
    SaveTranscoding = false;

    constructor(public setupService: SetupService, private mythService: MythService) {
        this.getJobQGlobal();
    }

    ngOnInit(): void {
        this.parent.children[this.tabIndex] = this;
    }

    dirty() {
        return this.currentForm.dirty;
    }

    ngAfterViewInit() {
    }

    getJobQGlobal() {

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "JobsRunOnRecordHost", Default: "0" })
            .subscribe({
                next: data => this.JobsRunOnRecordHost = (data.String == '1'),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AutoCommflagWhileRecording", Default: "0" })
            .subscribe({
                next: data => this.AutoCommflagWhileRecording = (data.String == '1'),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "JobQueueCommFlagCommand", Default: "mythcommflag" })
            .subscribe({
                next: data => this.JobQueueCommFlagCommand = data.String,
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "JobQueueTranscodeCommand", Default: "mythtranscode" })
            .subscribe({
                next: data => this.JobQueueTranscodeCommand = data.String,
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AutoTranscodeBeforeAutoCommflag", Default: "0" })
            .subscribe({
                next: data => this.AutoTranscodeBeforeAutoCommflag = (data.String == '1'),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "SaveTranscoding", Default: "0" })
            .subscribe({
                next: data => this.SaveTranscoding = (data.String == '1'),
                error: () => this.errorCount++
            });
    }

    JobQGlobalObs = {
        next: (x: any) => {
            if (x.bool)
                this.successCount++;
            else {
                this.errorCount++;
                if (this.currentForm)
                    this.currentForm.form.markAsDirty();
            }
        },
        error: (err: any) => {
            console.error(err);
            this.errorCount++;
            if (this.currentForm)
                this.currentForm.form.markAsDirty();
        },
    };

    markPristine() {
        setTimeout(() => {
            this.currentForm.form.markAsPristine();
            this.parent.showDirty();
        }, 100);
    }

    saveForm() {
        this.successCount = 0;
        this.errorCount = 0;

        this.successCount = 0;
        this.errorCount = 0;
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "JobsRunOnRecordHost",
            Value: this.JobsRunOnRecordHost ? "1" : "0"
        }).subscribe(this.JobQGlobalObs);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AutoCommflagWhileRecording",
            Value: this.AutoCommflagWhileRecording ? "1" : "0"
        }).subscribe(this.JobQGlobalObs);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "JobQueueCommFlagCommand",
            Value: this.JobQueueCommFlagCommand
        }).subscribe(this.JobQGlobalObs);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "JobQueueTranscodeCommand",
            Value: this.JobQueueTranscodeCommand
        }).subscribe(this.JobQGlobalObs);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AutoTranscodeBeforeAutoCommflag",
            Value: this.AutoTranscodeBeforeAutoCommflag ? "1" : "0"
        }).subscribe(this.JobQGlobalObs);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "SaveTranscoding",
            Value: this.SaveTranscoding ? "1" : "0"
        }).subscribe(this.JobQGlobalObs);
    }

}
