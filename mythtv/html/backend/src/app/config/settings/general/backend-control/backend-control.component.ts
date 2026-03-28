import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';

import { MythService } from 'src/app/services/myth.service';
import { SetupService } from 'src/app/services/setup.service';
import { TranslateModule } from '@ngx-translate/core';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';

import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { SettingsComponent } from '../general-settings.component';

@Component({
    selector: 'app-backend-control',
    templateUrl: './backend-control.component.html',
    styleUrls: ['./backend-control.component.css'],
    imports: [FormsModule, CardModule, SharedModule, MessageModule, ButtonModule, TranslateModule]
})
export class BackendControlComponent implements OnInit, AfterViewInit {

    @ViewChild("backendcontrol") currentForm!: NgForm;
    @Input() parent!: SettingsComponent;
    @Input() tabIndex!: number;

    successCount = 0;
    errorCount = 0;
    BackendStopCommand = "killall mythbackend";
    BackendStartCommand = "mythbackend";

    constructor(public setupService: SetupService, private mythService: MythService) { }

    ngOnInit(): void {
        this.parent.children[this.tabIndex] = this;
    }

    dirty() {
        return this.currentForm.dirty;
    }

    ngAfterViewInit() {
    }

    getBackendControl() {
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "BackendStopCommand", Default: "killall mythbackend" })
            .subscribe({
                next: data => this.BackendStopCommand = data.String,
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "BackendStartCommand", Default: "mythbackend" })
            .subscribe({
                next: data => this.BackendStartCommand = data.String,
                error: () => this.errorCount++
            });
    }

    becObserver = {
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
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "BackendStopCommand",
            Value: this.BackendStopCommand
        }).subscribe(this.becObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "BackendStartCommand",
            Value: this.BackendStartCommand
        }).subscribe(this.becObserver);
    }

}
